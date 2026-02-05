#include "storage_esp.h"
#include "../../enhance.h"
#include "../../globals/globals.h"
#include "../../gui/GUI.h"
#include <sdk/minecraft/minecraft.h>
#include <sdk/minecraft/world/world.h>
#include <sdk/minecraft/entity/entity.h>
#include <sdk/classloader.h>
#include <sdk/mappings/mappings.hpp>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>
#include <vector>
#include <mutex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static std::vector<storage_block_data> storage_blocks;
static storage_esp_camera_data storage_camera;
static std::mutex storage_blocks_mutex;

// Helper to process a single block entity and add to blocks_data if matching
static void process_block_entity(JNIEnv* env, jobject block_entity, jclass chest_class, jclass ender_chest_class, jclass shulker_class,
	double player_x, double player_y, double player_z, double max_range, std::vector<storage_block_data>& blocks_data)
{
	if (!block_entity) return;

	int type = -1;
	if (globals::storage_esp_chest && chest_class && env->IsInstanceOf(block_entity, chest_class))
		type = 0;
	else if (globals::storage_esp_ender_chest && ender_chest_class && env->IsInstanceOf(block_entity, ender_chest_class))
		type = 1;
	else if (globals::storage_esp_shulker && shulker_class && env->IsInstanceOf(block_entity, shulker_class))
		type = 2;

	if (type == -1) return;

	// Get position using getPos()
	jclass be_class = env->GetObjectClass(block_entity);
	jmethodID get_pos_mid = env->GetMethodID(be_class, sdk::mappings::block_entity_get_pos_name, sdk::mappings::block_entity_get_pos_sig);
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	if (!get_pos_mid)
	{
		get_pos_mid = env->GetMethodID(be_class, "getPos", "()Lnet/minecraft/class_2338;");
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	env->DeleteLocalRef(be_class);

	if (!get_pos_mid) return;

	jobject pos = env->CallObjectMethod(block_entity, get_pos_mid);
	if (!pos) return;

	// BlockPos extends Vec3i (class_2382), getX/Y/Z are in Vec3i
	// Vec3i intermediary: method_10263=getX, method_10264=getY, method_10260=getZ
	jclass pos_class = env->GetObjectClass(pos);
	
	// Try intermediary names first (Vec3i methods)
	jmethodID get_x_mid = env->GetMethodID(pos_class, "method_10263", "()I");
	if (env->ExceptionCheck()) env->ExceptionClear();
	jmethodID get_y_mid = env->GetMethodID(pos_class, "method_10264", "()I");
	if (env->ExceptionCheck()) env->ExceptionClear();
	jmethodID get_z_mid = env->GetMethodID(pos_class, "method_10260", "()I");
	if (env->ExceptionCheck()) env->ExceptionClear();
	
	// Fallback to named methods
	if (!get_x_mid)
	{
		get_x_mid = env->GetMethodID(pos_class, "getX", "()I");
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	if (!get_y_mid)
	{
		get_y_mid = env->GetMethodID(pos_class, "getY", "()I");
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	if (!get_z_mid)
	{
		get_z_mid = env->GetMethodID(pos_class, "getZ", "()I");
		if (env->ExceptionCheck()) env->ExceptionClear();
	}
	
	env->DeleteLocalRef(pos_class);

	if (!get_x_mid || !get_y_mid || !get_z_mid)
	{
		env->DeleteLocalRef(pos);
		return;
	}

	int bx = env->CallIntMethod(pos, get_x_mid);
	int by = env->CallIntMethod(pos, get_y_mid);
	int bz = env->CallIntMethod(pos, get_z_mid);
	env->DeleteLocalRef(pos);

	double dx = bx + 0.5 - player_x;
	double dy = by + 0.5 - player_y;
	double dz = bz + 0.5 - player_z;
	double dist = sqrt(dx * dx + dy * dy + dz * dz);
	if (dist > max_range) return;

	storage_block_data data;
	data.x = bx + 0.5;
	data.y = by + 0.5;
	data.z = bz + 0.5;
	data.type = type;
	blocks_data.push_back(data);
}

void enhance::modules::storage_esp::run()
{
	try
	{
		if (!globals::storage_esp_enabled)
		{
			std::lock_guard<std::mutex> lock(storage_blocks_mutex);
			storage_blocks.clear();
			return;
		}

		if (!globals::storage_esp_chest && !globals::storage_esp_ender_chest && !globals::storage_esp_shulker)
		{
			std::lock_guard<std::mutex> lock(storage_blocks_mutex);
			storage_blocks.clear();
			return;
		}

		jobject world = sdk::instance->get_world();
		if (!world) return;

		jobject local_player = sdk::instance->get_player();
		if (!local_player)
		{
			auto env = enhance::instance->get_env();
			if (env) env->DeleteLocalRef(world);
			return;
		}

		auto env = enhance::instance->get_env();
		if (!env) return;

		// Get camera data
		sdk::camera_data cam = sdk::instance->get_camera();
		if (!cam.valid)
		{
			sdk::entity_client local_entity_client(local_player);
			cam.x = local_entity_client.get_x();
			cam.y = local_entity_client.get_y() + 1.62;
			cam.z = local_entity_client.get_z();
			cam.yaw = local_entity_client.get_yaw();
			cam.pitch = local_entity_client.get_pitch();
			cam.fov = 70.0f;
		}

		storage_esp_camera_data camera;
		camera.cam_x = cam.x;
		camera.cam_y = cam.y;
		camera.cam_z = cam.z;
		camera.yaw = cam.yaw;
		camera.pitch = cam.pitch;
		camera.fov = cam.fov;

		std::vector<storage_block_data> blocks_data;

		// Get player position
		sdk::entity_client local_entity(local_player);
		double player_x = local_entity.get_x();
		double player_y = local_entity.get_y();
		double player_z = local_entity.get_z();
		const double MAX_RANGE = 64.0;

		// Find block entity classes
		jclass chest_class = sdk::classloader::find_class(env, sdk::mappings::chest_block_entity_class_sig);
		jclass ender_chest_class = sdk::classloader::find_class(env, sdk::mappings::ender_chest_block_entity_class_sig);
		jclass shulker_class = sdk::classloader::find_class(env, sdk::mappings::shulker_box_block_entity_class_sig);

		// Scan blocks around player using getBlockState approach
		jclass world_class = env->GetObjectClass(world);
		
		// Get BlockPos class and constructor
		jclass block_pos_class = sdk::classloader::find_class(env, "net/minecraft/class_2338");
		if (!block_pos_class)
		{
			env->DeleteLocalRef(world_class);
			env->DeleteLocalRef(world);
			env->DeleteLocalRef(local_player);
			if (chest_class) env->DeleteLocalRef(chest_class);
			if (ender_chest_class) env->DeleteLocalRef(ender_chest_class);
			if (shulker_class) env->DeleteLocalRef(shulker_class);
			return;
		}
		
		// BlockPos constructor (int, int, int) - inherited from Vec3i
		jmethodID block_pos_ctor = env->GetMethodID(block_pos_class, "<init>", "(III)V");
		if (env->ExceptionCheck()) env->ExceptionClear();
		
		// World.getBlockState(BlockPos) -> BlockState
		jmethodID get_block_state_mid = nullptr;
		if (sdk::mappings::world_get_block_state_name && sdk::mappings::world_get_block_state_sig)
		{
			get_block_state_mid = env->GetMethodID(world_class, sdk::mappings::world_get_block_state_name, sdk::mappings::world_get_block_state_sig);
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		// Fallback to intermediary name if mappings not available
		if (!get_block_state_mid)
		{
			// method_8320 is getBlockState in World (intermediary)
			get_block_state_mid = env->GetMethodID(world_class, "method_8320", "(Lnet/minecraft/class_2338;)Lnet/minecraft/class_2680;");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		if (!get_block_state_mid)
		{
			get_block_state_mid = env->GetMethodID(world_class, "getBlockState", "(Lnet/minecraft/class_2338;)Lnet/minecraft/class_2680;");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		// BlockState.getBlock() -> Block
		jclass block_state_class = nullptr;
		jmethodID get_block_mid = nullptr;
		
		// Try using mappings first
		if (sdk::mappings::block_state_class_sig)
		{
			block_state_class = sdk::classloader::find_class(env, sdk::mappings::block_state_class_sig);
		}
		
		// Fallback to hardcoded class signature
		if (!block_state_class)
		{
			block_state_class = sdk::classloader::find_class(env, "net/minecraft/class_2680");
		}
		
		if (block_state_class)
		{
			if (sdk::mappings::block_state_get_block_name && sdk::mappings::block_state_get_block_sig)
			{
				get_block_mid = env->GetMethodID(block_state_class, sdk::mappings::block_state_get_block_name, sdk::mappings::block_state_get_block_sig);
				if (env->ExceptionCheck()) env->ExceptionClear();
			}
			
			// Fallback to intermediary name if mappings not available
			if (!get_block_mid)
			{
				// method_26204 is getBlock in BlockState (intermediary)
				get_block_mid = env->GetMethodID(block_state_class, "method_26204", "()Lnet/minecraft/class_2248;");
				if (env->ExceptionCheck()) env->ExceptionClear();
			}
			
			if (!get_block_mid)
			{
				get_block_mid = env->GetMethodID(block_state_class, "getBlock", "()Lnet/minecraft/class_2248;");
				if (env->ExceptionCheck()) env->ExceptionClear();
			}
		}
		
		// Storage block classes - use parent classes to match Java implementation
		// AbstractChestBlock covers both ChestBlock and EnderChestBlock
		jclass abstract_chest_block_class = sdk::classloader::find_class(env, "net/minecraft/class_2226"); // AbstractChestBlock
		if (!abstract_chest_block_class)
		{
			abstract_chest_block_class = sdk::classloader::find_class(env, "net/minecraft/block/AbstractChestBlock");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		jclass barrel_block_class = sdk::classloader::find_class(env, "net/minecraft/class_2203"); // BarrelBlock
		if (!barrel_block_class)
		{
			barrel_block_class = sdk::classloader::find_class(env, "net/minecraft/block/BarrelBlock");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		jclass shulker_box_block_class = sdk::classloader::find_class(env, "net/minecraft/class_2478"); // ShulkerBoxBlock
		if (!shulker_box_block_class)
		{
			shulker_box_block_class = sdk::classloader::find_class(env, "net/minecraft/block/ShulkerBoxBlock");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		jclass hopper_block_class = sdk::classloader::find_class(env, "net/minecraft/class_2265"); // HopperBlock
		if (!hopper_block_class)
		{
			hopper_block_class = sdk::classloader::find_class(env, "net/minecraft/block/HopperBlock");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		jclass dispenser_block_class = sdk::classloader::find_class(env, "net/minecraft/class_2272"); // DispenserBlock
		if (!dispenser_block_class)
		{
			dispenser_block_class = sdk::classloader::find_class(env, "net/minecraft/block/DispenserBlock");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		jclass crafter_block_class = sdk::classloader::find_class(env, "net/minecraft/class_2290"); // CrafterBlock (might not exist in 1.21.10)
		if (!crafter_block_class)
		{
			crafter_block_class = sdk::classloader::find_class(env, "net/minecraft/block/CrafterBlock");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		jclass abstract_furnace_block_class = sdk::classloader::find_class(env, "net/minecraft/class_2283"); // AbstractFurnaceBlock
		if (!abstract_furnace_block_class)
		{
			abstract_furnace_block_class = sdk::classloader::find_class(env, "net/minecraft/block/AbstractFurnaceBlock");
			if (env->ExceptionCheck()) env->ExceptionClear();
		}
		
		if (block_pos_ctor && get_block_state_mid && get_block_mid)
		{
			int scan_radius = 32; // Scan 32 blocks around player
			int min_y = -64;
			int max_y = 320;
			
			int px = (int)floor(player_x);
			int py = (int)floor(player_y);
			int pz = (int)floor(player_z);
			
			// Limit Y range to reasonable area around player
			int start_y = (min_y > py - 20) ? min_y : (py - 20);
			int end_y = (max_y < py + 20) ? max_y : (py + 20);
			
			int blocks_scanned = 0;
			
			for (int x = px - scan_radius; x <= px + scan_radius; x += 1)
			{
				for (int z = pz - scan_radius; z <= pz + scan_radius; z += 1)
				{
					for (int y = start_y; y <= end_y; y += 1)
					{
						// Create BlockPos
						jobject pos = env->NewObject(block_pos_class, block_pos_ctor, x, y, z);
						if (!pos) continue;
						
						// Get block state
						jobject block_state = env->CallObjectMethod(world, get_block_state_mid, pos);
						env->DeleteLocalRef(pos);
						
						if (!block_state) continue;
						blocks_scanned++;
						
						// Get block
						jobject block = env->CallObjectMethod(block_state, get_block_mid);
						env->DeleteLocalRef(block_state);
						
						if (!block) continue;
						
						// Check if it's a storage block - match Java implementation
						// block instanceof AbstractChestBlock || block instanceof BarrelBlock || 
						// block instanceof ShulkerBoxBlock || block instanceof HopperBlock || 
						// block instanceof DispenserBlock || block instanceof CrafterBlock ||
						// block instanceof AbstractFurnaceBlock
						int type = -1;
						
						// Check AbstractChestBlock (covers both chest and ender chest)
						if (abstract_chest_block_class && env->IsInstanceOf(block, abstract_chest_block_class))
						{
							// For chest/ender chest, prefer chest type (0) if enabled, else ender chest (1)
							if (globals::storage_esp_chest)
								type = 0;
							else if (globals::storage_esp_ender_chest)
								type = 1;
						}
						else if (barrel_block_class && env->IsInstanceOf(block, barrel_block_class))
						{
							if (globals::storage_esp_chest)
								type = 0;
						}
						else if (globals::storage_esp_shulker && shulker_box_block_class && env->IsInstanceOf(block, shulker_box_block_class))
						{
							type = 2;
						}
						else if (hopper_block_class && env->IsInstanceOf(block, hopper_block_class))
						{
							if (globals::storage_esp_chest)
								type = 0;
						}
						else if (dispenser_block_class && env->IsInstanceOf(block, dispenser_block_class))
						{
							if (globals::storage_esp_chest)
								type = 0;
						}
						else if (crafter_block_class && env->IsInstanceOf(block, crafter_block_class))
						{
							if (globals::storage_esp_chest)
								type = 0;
						}
						else if (abstract_furnace_block_class && env->IsInstanceOf(block, abstract_furnace_block_class))
						{
							if (globals::storage_esp_chest)
								type = 0;
						}
						
						env->DeleteLocalRef(block);
						
						if (type != -1)
						{
							storage_block_data data;
							data.x = x + 0.5;
							data.y = y + 0.5;
							data.z = z + 0.5;
							data.type = type;
							blocks_data.push_back(data);
						}
					}
				}
			}
		}
		
		if (abstract_chest_block_class) env->DeleteLocalRef(abstract_chest_block_class);
		if (barrel_block_class) env->DeleteLocalRef(barrel_block_class);
		if (shulker_box_block_class) env->DeleteLocalRef(shulker_box_block_class);
		if (hopper_block_class) env->DeleteLocalRef(hopper_block_class);
		if (dispenser_block_class) env->DeleteLocalRef(dispenser_block_class);
		if (crafter_block_class) env->DeleteLocalRef(crafter_block_class);
		if (abstract_furnace_block_class) env->DeleteLocalRef(abstract_furnace_block_class);
		if (block_state_class) env->DeleteLocalRef(block_state_class);
		env->DeleteLocalRef(block_pos_class);

		env->DeleteLocalRef(world_class);
		if (chest_class) env->DeleteLocalRef(chest_class);
		if (ender_chest_class) env->DeleteLocalRef(ender_chest_class);
		if (shulker_class) env->DeleteLocalRef(shulker_class);
		env->DeleteLocalRef(world);
		env->DeleteLocalRef(local_player);

		{
			std::lock_guard<std::mutex> lock(storage_blocks_mutex);
			storage_blocks = blocks_data;
			storage_camera = camera;
		}
	}
	catch (...)
	{
	}
}

// View matrix for world-to-screen
struct storage_view_matrix_t
{
	float right[3];
	float up[3];
	float forward[3];
	float cam_pos[3];
	float fov_x, fov_y;
	float half_w, half_h;
	bool valid;
};

static storage_view_matrix_t g_storage_view_matrix;

static void compute_storage_view_matrix(float cam_x, float cam_y, float cam_z, float yaw, float pitch, float fov, int screen_width, int screen_height)
{
	float yaw_rad = yaw * (float)(M_PI / 180.0);
	float pitch_rad = pitch * (float)(M_PI / 180.0);

	float cos_yaw = cosf(yaw_rad);
	float sin_yaw = sinf(yaw_rad);
	float cos_pitch = cosf(pitch_rad);
	float sin_pitch = sinf(pitch_rad);

	g_storage_view_matrix.forward[0] = -sin_yaw * cos_pitch;
	g_storage_view_matrix.forward[1] = -sin_pitch;
	g_storage_view_matrix.forward[2] = cos_yaw * cos_pitch;

	g_storage_view_matrix.right[0] = cos_yaw;
	g_storage_view_matrix.right[1] = 0.0f;
	g_storage_view_matrix.right[2] = sin_yaw;

	g_storage_view_matrix.up[0] = sin_yaw * sin_pitch;
	g_storage_view_matrix.up[1] = cos_pitch;
	g_storage_view_matrix.up[2] = -cos_yaw * sin_pitch;

	g_storage_view_matrix.cam_pos[0] = cam_x;
	g_storage_view_matrix.cam_pos[1] = cam_y;
	g_storage_view_matrix.cam_pos[2] = cam_z;

	float fov_rad = fov * (float)(M_PI / 180.0);
	float aspect = (float)screen_width / (float)screen_height;
	float tan_half_fov = tanf(fov_rad * 0.5f);
	g_storage_view_matrix.fov_x = 1.0f / (tan_half_fov * aspect);
	g_storage_view_matrix.fov_y = 1.0f / tan_half_fov;

	g_storage_view_matrix.half_w = screen_width * 0.5f;
	g_storage_view_matrix.half_h = screen_height * 0.5f;

	g_storage_view_matrix.valid = true;
}

static inline bool storage_world_to_screen(float x, float y, float z, float& out_x, float& out_y)
{
	float lx = x - g_storage_view_matrix.cam_pos[0];
	float ly = y - g_storage_view_matrix.cam_pos[1];
	float lz = z - g_storage_view_matrix.cam_pos[2];

	float px = lx * g_storage_view_matrix.right[0] + ly * g_storage_view_matrix.right[1] + lz * g_storage_view_matrix.right[2];
	float py = lx * g_storage_view_matrix.up[0] + ly * g_storage_view_matrix.up[1] + lz * g_storage_view_matrix.up[2];
	float pz = lx * g_storage_view_matrix.forward[0] + ly * g_storage_view_matrix.forward[1] + lz * g_storage_view_matrix.forward[2];

	if (pz < 0.01f) return false;
	
	float inv_z = 1.0f / pz;
	out_x = g_storage_view_matrix.half_w + (px * inv_z * g_storage_view_matrix.fov_x) * g_storage_view_matrix.half_w;
	out_y = g_storage_view_matrix.half_h - (py * inv_z * g_storage_view_matrix.fov_y) * g_storage_view_matrix.half_h;
	
	return true;
}

void enhance::modules::storage_esp::draw_boxes()
{
	if (!globals::storage_esp_enabled) return;
	if (!GUI::get_is_init()) return;

	ImGuiIO& io = ImGui::GetIO();
	int screen_width = (int)io.DisplaySize.x;
	int screen_height = (int)io.DisplaySize.y;
	if (screen_width <= 0 || screen_height <= 0) return;

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
	if (!draw_list) return;

	std::vector<storage_block_data> blocks_copy;
	storage_esp_camera_data camera;
	{
		std::lock_guard<std::mutex> lock(storage_blocks_mutex);
		if (storage_blocks.empty()) return;
		blocks_copy = storage_blocks;
		camera = storage_camera;
	}

	compute_storage_view_matrix((float)camera.cam_x, (float)camera.cam_y, (float)camera.cam_z,
	                            camera.yaw, camera.pitch, camera.fov, screen_width, screen_height);

	// Colors for different container types
	const ImU32 chest_color = IM_COL32(200, 150, 50, 255);       // Orange/brown for chests
	const ImU32 ender_chest_color = IM_COL32(128, 50, 200, 255); // Purple for ender chests
	const ImU32 shulker_color = IM_COL32(200, 100, 200, 255);    // Pink for shulkers
	const ImU32 black_color = IM_COL32(0, 0, 0, 255);

	for (const auto& block : blocks_copy)
	{
		// Block corners (1x1x1 block)
		float half = 0.5f;
		float corners[8][3] = {
			{(float)block.x - half, (float)block.y - half, (float)block.z - half},
			{(float)block.x + half, (float)block.y - half, (float)block.z - half},
			{(float)block.x + half, (float)block.y - half, (float)block.z + half},
			{(float)block.x - half, (float)block.y - half, (float)block.z + half},
			{(float)block.x - half, (float)block.y + half, (float)block.z - half},
			{(float)block.x + half, (float)block.y + half, (float)block.z - half},
			{(float)block.x + half, (float)block.y + half, (float)block.z + half},
			{(float)block.x - half, (float)block.y + half, (float)block.z + half}
		};

		float screen_x[8], screen_y[8];
		int valid_count = 0;
		float min_x = FLT_MAX, max_x = -FLT_MAX;
		float min_y = FLT_MAX, max_y = -FLT_MAX;

		for (int i = 0; i < 8; i++)
		{
			if (storage_world_to_screen(corners[i][0], corners[i][1], corners[i][2], screen_x[i], screen_y[i]))
			{
				if (screen_x[i] < min_x) min_x = screen_x[i];
				if (screen_x[i] > max_x) max_x = screen_x[i];
				if (screen_y[i] < min_y) min_y = screen_y[i];
				if (screen_y[i] > max_y) max_y = screen_y[i];
				valid_count++;
			}
		}

		if (valid_count < 4) continue;

		// Select color based on type
		ImU32 box_color;
		switch (block.type)
		{
			case 0: box_color = chest_color; break;
			case 1: box_color = ender_chest_color; break;
			case 2: box_color = shulker_color; break;
			default: box_color = chest_color; break;
		}

		// Draw box with outline
		draw_list->AddRect(ImVec2(min_x - 1, min_y - 1), ImVec2(max_x + 1, max_y + 1), black_color, 0.0f, 0, 1.0f);
		draw_list->AddRect(ImVec2(min_x, min_y), ImVec2(max_x, max_y), box_color, 0.0f, 0, 1.0f);
		draw_list->AddRect(ImVec2(min_x + 1, min_y + 1), ImVec2(max_x - 1, max_y - 1), black_color, 0.0f, 0, 1.0f);
	}
}
