#ifndef MAPPINGS_HPP
#define MAPPINGS_HPP
#include <memory>

#include <string>
namespace sdk
{
	namespace mappings
	{
		static const char* version = "1.21.10";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html
		static const char* minecraftclass_sig = "net/minecraft/class_310";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#instance
		static const char* minecraftclient_name = "field_1700";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#instance
		static const char* minecraftclient_sig = "Lnet/minecraft/class_310;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#player
		static const char* player_name = "field_1724";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#player
		static const char* player_sig = "Lnet/minecraft/class_746;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#world
		static const char* world_name = "field_1687";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#world
		static const char* world_sig = "Lnet/minecraft/class_638;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#crosshairTarget
		static const char* crosshair_target_name = "field_1765";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#crosshairTarget
		static const char* crosshair_target_sig = "Lnet/minecraft/class_239;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#interactionManager
		static const char* interaction_manager_name = "field_1761";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#interactionManager
		static const char* interaction_manager_sig = "Lnet/minecraft/class_636;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#getNetworkHandler()
		static const char* network_handler_name = "method_1562";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#getNetworkHandler()
		static const char* network_handler_sig = "()Lnet/minecraft/class_634;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#connection
		static const char* connection_name = "field_1746";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#connection
		static const char* connection_sig = "Lnet/minecraft/class_2535;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#doAttack()
		static const char* do_attack_name = "method_1536";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#doAttack()
		static const char* do_attack_sig = "()Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#attackCooldown
		static const char* attack_cooldown_name = "field_1771";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#attackCooldown
		static const char* attack_cooldown_sig = "I";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/GameRenderer.html
		static const char* gamerenderer_class_sig = "net/minecraft/class_757";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#gameRenderer
		static const char* gamerenderer_name = "field_1773";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/MinecraftClient.html#gameRenderer
		static const char* gamerenderer_sig = "Lnet/minecraft/class_757;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/GameRenderer.html#updateCrosshairTarget(float)
		static const char* update_crosshair_target_name = "method_3190";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/GameRenderer.html#updateCrosshairTarget(float)
		static const char* update_crosshair_target_sig = "(F)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/GameRenderer.html#getFov(net.minecraft.client.render.Camera,float,boolean)
		static const char* get_fov_name = "method_3196";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/GameRenderer.html#getFov(net.minecraft.client.render.Camera,float,boolean)
		static const char* get_fov_sig = "(Lnet/minecraft/class_4184;FZ)F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/GameRenderer.html#getCamera()
		static const char* get_camera_name = "method_19418";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/GameRenderer.html#getCamera()
		static const char* get_camera_sig = "()Lnet/minecraft/class_4184;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/Camera.html
		static const char* camera_class_sig = "net/minecraft/class_4184";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/Camera.html#getPos()
		static const char* camera_get_pos_name = "method_19326";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/Camera.html#getPos()
		static const char* camera_get_pos_sig = "()Lnet/minecraft/class_243;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/Camera.html#getYaw()
		static const char* camera_get_yaw_name = "method_19330";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/Camera.html#getYaw()
		static const char* camera_get_yaw_sig = "()F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/Camera.html#getPitch()
		static const char* camera_get_pitch_name = "method_19329";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/render/Camera.html#getPitch()
		static const char* camera_get_pitch_sig = "()F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Vec3d.html
		static const char* vec3d_class_sig = "net/minecraft/class_243";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Vec3d.html#x
		static const char* vec3d_x_name = "field_1352";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Vec3d.html#x
		static const char* vec3d_x_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Vec3d.html#y
		static const char* vec3d_y_name = "field_1351";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Vec3d.html#y
		static const char* vec3d_y_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Vec3d.html#z
		static const char* vec3d_z_name = "field_1350";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Vec3d.html#z
		static const char* vec3d_z_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerEntity.html
		static const char* clientplayerentity_class_sig = "net/minecraft/class_746";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerEntity.html#sendMovementPackets()
		static const char* send_movement_packets_name = "method_3136";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerEntity.html#sendMovementPackets()
		static const char* send_movement_packets_sig = "()V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerEntity.html#renderYaw
		static const char* render_yaw_name = "field_3932";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerEntity.html#renderYaw
		static const char* render_yaw_sig = "F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerEntity.html#renderPitch
		static const char* render_pitch_name = "field_3916";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerEntity.html#renderPitch
		static const char* render_pitch_sig = "F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerMoveC2SPacket.html
		static const char* playermovec2spacket_class_sig = "net/minecraft/class_2828";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerMoveC2SPacket.html#yaw
		static const char* playermovec2spacket_yaw_name = "field_12887";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerMoveC2SPacket.html#yaw
		static const char* playermovec2spacket_yaw_sig = "F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerMoveC2SPacket.html#pitch
		static const char* playermovec2spacket_pitch_name = "field_12885";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerMoveC2SPacket.html#pitch
		static const char* playermovec2spacket_pitch_sig = "F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#abilities
		static const char* abilities_name = "field_7503";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#abilities
		static const char* abilities_sig = "Lnet/minecraft/class_1656;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#getEntityInteractionRange()
		static const char* get_entity_interaction_range_name = "method_55755";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#getEntityInteractionRange()
		static const char* get_entity_interaction_range_sig = "()D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#getAttackCooldownProgress(float)
		static const char* get_attack_cooldown_progress_name = "method_7261";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#getAttackCooldownProgress(float)
		static const char* get_attack_cooldown_progress_sig = "(F)F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html
		static const char* player_entity_class_sig = "net/minecraft/class_1657";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerAbilities.html#flying
		static const char* fly_name = "field_7479";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerAbilities.html#flying
		static const char* fly_sig = "Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getX()
		static const char* entity_get_x_name = "method_23317";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getX()
		static const char* entity_get_x_sig = "()D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getY()
		static const char* entity_get_y_name = "method_23318";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getY()
		static const char* entity_get_y_sig = "()D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getZ()
		static const char* entity_get_z_name = "method_23321";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getZ()
		static const char* entity_get_z_sig = "()D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getYaw()
		static const char* entity_get_yaw_name = "method_36454";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getYaw()
		static const char* entity_get_yaw_sig = "()F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getPitch()
		static const char* entity_get_pitch_name = "method_5695";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getPitch()
		static const char* entity_get_pitch_sig = "(F)F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setYaw(float)
		static const char* entity_set_yaw_name = "method_36456";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setYaw(float)
		static const char* entity_set_yaw_sig = "(F)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setPitch(float)
		static const char* entity_set_pitch_name = "method_36457";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setPitch(float)
		static const char* entity_set_pitch_sig = "(F)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getBoundingBox()
		static const char* get_bounding_box_name = "field_6005";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#getBoundingBox()
		static const char* get_bounding_box_sig = "Lnet/minecraft/class_238;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setBoundingBox(net.minecraft.util.math.Box)
		static const char* set_bounding_box_name = "method_5857";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setBoundingBox(net.minecraft.util.math.Box)
		static const char* set_bounding_box_sig = "(Lnet/minecraft/class_238;)Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html
		static const char* entity_class_sig = "net/minecraft/class_1297";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setFlag(int,boolean)
		static const char* entity_set_flag_name = "method_5729";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setFlag(int,boolean)
		static const char* entity_set_flag_sig = "(IZ)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#minX
		static const char* box_min_x_name = "field_1323";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#minX
		static const char* box_min_x_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#maxX
		static const char* box_max_x_name = "field_1320";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#maxX
		static const char* box_max_x_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#minY
		static const char* box_min_y_name = "field_1322";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#minY
		static const char* box_min_y_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#maxY
		static const char* box_max_y_name = "field_1325";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#maxY
		static const char* box_max_y_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#minZ
		static const char* box_min_z_name = "field_1321";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#minZ
		static const char* box_min_z_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#maxZ
		static const char* box_max_z_name = "field_1324";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Box.html#maxZ
		static const char* box_max_z_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/world/ClientWorld.html#players
		static const char* players_field_name = "field_18226";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/world/ClientWorld.html#players
		static const char* players_field_sig = "Ljava/util/List;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html
		static const char* living_entity_class_sig = "net/minecraft/class_1309";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#isBlocking()
		static const char* living_entity_is_blocking_name = "method_6039";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#isBlocking()
		static const char* living_entity_is_blocking_sig = "()Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#setSprinting(boolean)
		static const char* set_sprinting_name = "method_5728";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#setSprinting(boolean)
		static const char* set_sprinting_sig = "(Z)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#isSprinting()
		static const char* is_sprinting_name = "method_5624";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#isSprinting()
		static const char* is_sprinting_sig = "()Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#isOnGround()
		static const char* is_on_ground_name = "method_24828";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#isOnGround()
		static const char* is_on_ground_sig = "()Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setVelocity(net.minecraft.util.math.Vec3d)
		static const char* entity_set_velocity_name = "method_18800";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#setVelocity(net.minecraft.util.math.Vec3d)
		static const char* entity_set_velocity_sig = "(DDD)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#velocity
		static const char* entity_velocity_name = "field_18276";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#velocity
		static const char* entity_velocity_sig = "Lnet/minecraft/class_243;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#handleFallDamage(double,float,net.minecraft.entity.damage.DamageSource)
		static const char* entity_handle_fall_damage_name = "method_5747";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#handleFallDamage(double,float,net.minecraft.entity.damage.DamageSource)
		static const char* entity_handle_fall_damage_sig = "(DFLnet/minecraft/class_1282;)Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#fallDistance
		static const char* entity_fall_distance_name = "field_6017";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/Entity.html#fallDistance
		static const char* entity_fall_distance_sig = "D";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#attack(net.minecraft.entity.Entity)
		static const char* player_attack_name = "method_7324";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#attack(net.minecraft.entity.Entity)
		static const char* player_attack_sig = "(Lnet/minecraft/class_1297;)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#attackLivingEntity(net.minecraft.entity.LivingEntity)
		static const char* player_attack_living_entity_name = "method_5997";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#attackLivingEntity(net.minecraft.entity.LivingEntity)
		static const char* player_attack_living_entity_sig = "(Lnet/minecraft/class_1309;)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#inventory
		static const char* player_inventory_name = "field_7514";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerEntity.html#inventory
		static const char* player_inventory_sig = "Lnet/minecraft/class_1661;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html
		static const char* player_inventory_class_sig = "net/minecraft/class_1661";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#selectedSlot
		static const char* inventory_selected_slot_name = "field_7545";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#selectedSlot
		static const char* inventory_selected_slot_sig = "I";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#getStack(int)
		static const char* inventory_get_stack_name = "method_5438";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#getStack(int)
		static const char* inventory_get_stack_sig = "(I)Lnet/minecraft/class_1799;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#setStack(int,net.minecraft.item.ItemStack)
		static const char* inventory_set_stack_name = "method_5447";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#setStack(int,net.minecraft.item.ItemStack)
		static const char* inventory_set_stack_sig = "(ILnet/minecraft/class_1799;)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/item/ItemStack.html#getItem()
		static const char* itemstack_get_item_name = "method_7909";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/item/ItemStack.html#getItem()
		static const char* itemstack_get_item_sig = "()Lnet/minecraft/class_1792;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/item/ItemStack.html#isEmpty()
		static const char* itemstack_is_empty_name = "method_7960";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/item/ItemStack.html#isEmpty()
		static const char* itemstack_is_empty_sig = "()Z";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/item/Item.html
		static const char* item_class_sig = "net/minecraft/class_1792";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/item/Item.html#getTranslationKey()
		static const char* item_get_translation_key_name = "method_7876";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/item/Item.html#getTranslationKey()
		static const char* item_get_translation_key_sig = "()Ljava/lang/String;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerInteractionManager.html#interactItem(net.minecraft.entity.player.PlayerEntity,net.minecraft.util.Hand)
		static const char* interact_item_name = "method_2919";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayerInteractionManager.html#interactItem(net.minecraft.entity.player.PlayerEntity,net.minecraft.util.Hand)
		static const char* interact_item_sig = "(Lnet/minecraft/class_1657;Lnet/minecraft/class_1268;)Lnet/minecraft/class_1269;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/Hand.html
		static const char* hand_class_sig = "net/minecraft/class_1268";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/Hand.html#MAIN_HAND
		static const char* hand_main_hand_name = "field_5808";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/Hand.html#MAIN_HAND
		static const char* hand_main_hand_sig = "Lnet/minecraft/class_1268;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/EntityHitResult.html
		static const char* entity_hit_result_class_sig = "net/minecraft/class_3966";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/EntityHitResult.html#getEntity()
		static const char* entity_hit_result_get_entity_name = "method_17782";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/EntityHitResult.html#getEntity()
		static const char* entity_hit_result_get_entity_sig = "()Lnet/minecraft/class_1297;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#getHealth()
		static const char* living_entity_get_health_name = "method_6032";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#getHealth()
		static const char* living_entity_get_health_sig = "()F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#getMaxHealth()
		static const char* living_entity_get_max_health_name = "method_6063";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#getMaxHealth()
		static const char* living_entity_get_max_health_sig = "()F";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#hurtTime
		static const char* living_entity_hurt_time_name = "field_6235";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/LivingEntity.html#hurtTime
		static const char* living_entity_hurt_time_sig = "I";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#offHand
		static const char* inventory_offhand_name = "field_30639";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/entity/player/PlayerInventory.html#offHand
		static const char* inventory_offhand_sig = "I";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/world/ClientWorld.html
		static const char* client_world_class_sig = "net/minecraft/class_638";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/world/ClientWorld.html#blockEntities
		static const char* client_world_block_entities_name = "field_60919";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/world/ClientWorld.html#blockEntities
		static const char* client_world_block_entities_sig = "Ljava/util/Set;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/entity/BlockEntity.html
		static const char* block_entity_class_sig = "net/minecraft/class_2586";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/entity/BlockEntity.html#getPos()
		static const char* block_entity_get_pos_name = "method_11016";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/entity/BlockEntity.html#getPos()
		static const char* block_entity_get_pos_sig = "()Lnet/minecraft/class_2338;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html
		static const char* block_pos_class_sig = "net/minecraft/class_2338";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#getX()
		static const char* block_pos_get_x_name = "method_10263";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#getX()
		static const char* block_pos_get_x_sig = "()I";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#getY()
		static const char* block_pos_get_y_name = "method_10264";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#getY()
		static const char* block_pos_get_y_sig = "()I";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#getZ()
		static const char* block_pos_get_z_name = "method_10260";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#getZ()
		static const char* block_pos_get_z_sig = "()I";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/world/World.html
		static const char* world_class_sig = "net/minecraft/class_1937";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/world/World.html#getBlockState(net.minecraft.util.math.BlockPos)
		static const char* world_get_block_state_name = "method_66016";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/world/World.html#getBlockState(net.minecraft.util.math.BlockPos)
		static const char* world_get_block_state_sig = "(Lnet/minecraft/class_2338;Lnet/minecraft/class_2680;Lnet/minecraft/class_2680;)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/AbstractBlock.AbstractBlockState.html
		static const char* block_state_class_sig = "net/minecraft/class_4970$class_4971";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/AbstractBlock.AbstractBlockState.html#getBlock()
		static const char* block_state_get_block_name = "method_26204";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/AbstractBlock.AbstractBlockState.html#getBlock()
		static const char* block_state_get_block_sig = "()Lnet/minecraft/class_2248;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/entity/ChestBlockEntity.html
		static const char* chest_block_entity_class_sig = "net/minecraft/class_2595";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/entity/EnderChestBlockEntity.html
		static const char* ender_chest_block_entity_class_sig = "net/minecraft/class_2611";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/entity/ShulkerBoxBlockEntity.html
		static const char* shulker_box_block_entity_class_sig = "net/minecraft/class_2627";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/BlockHitResult.html
		static const char* block_hit_result_class_sig = "net/minecraft/class_3965";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/BlockHitResult.html#getBlockPos()
		static const char* block_hit_result_get_block_pos_name = "method_17777";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/BlockHitResult.html#getBlockPos()
		static const char* block_hit_result_get_block_pos_sig = "()Lnet/minecraft/class_2338;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/HitResult.html#getType()
		static const char* block_hit_result_get_type_name = "method_17783";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/hit/HitResult.html#getType()
		static const char* block_hit_result_get_type_sig = "()Lnet/minecraft/class_239$class_240;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/block/ObsidianBlock.html
		static const char* obsidian_block_class_sig = "net/minecraft/class_663$class_11926";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayNetworkHandler.html
		static const char* network_handler_class_sig = "net/minecraft/class_634";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayNetworkHandler.html#sendPacket(net.minecraft.network.Packet)
		static const char* send_packet_name = "method_45729";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/client/network/ClientPlayNetworkHandler.html#sendPacket(net.minecraft.network.Packet)
		static const char* send_packet_sig = "(Ljava/lang/String;)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/UpdateSelectedSlotC2SPacket.html
		static const char* update_selected_slot_c2s_packet_class_sig = "net/minecraft/class_2868";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerActionC2SPacket.html
		static const char* player_action_c2s_packet_class_sig = "net/minecraft/class_2846";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerActionC2SPacket$Action.html
		static const char* player_action_c2s_packet_action_class_sig = "net/minecraft/class_2846$class_2847";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerActionC2SPacket$Action.html#SWAP_ITEM_WITH_OFFHAND
		static const char* swap_item_with_offhand_action_name = "field_12969";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/PlayerActionC2SPacket$Action.html#SWAP_ITEM_WITH_OFFHAND
		static const char* swap_item_with_offhand_action_sig = "Lnet/minecraft/class_2846$class_2847;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/c2s/play/UpdateSelectedSlotC2SPacket.html
		static const char* pick_from_inventory_c2s_packet_class_sig = "net/minecraft/class_2868";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#ORIGIN
		static const char* block_pos_origin_name = "field_10980";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/BlockPos.html#ORIGIN
		static const char* block_pos_origin_sig = "Lnet/minecraft/class_2338;";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Direction.html
		static const char* direction_class_sig = "net/minecraft/class_2350";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Direction.html#DOWN
		static const char* direction_down_name = "field_11033";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/util/math/Direction.html#DOWN
		static const char* direction_down_sig = "Lnet/minecraft/class_2350;";
		static const char* channel_inbound_handler_adapter_class_sig = "io/netty/channel/ChannelInboundHandlerAdapter";
		static const char* channel_read0_name = "channelRead0";
		static const char* channel_read0_sig = "(Lio/netty/channel/ChannelHandlerContext;Ljava/lang/Object;)V";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/s2c/play/EntityS2CPacket.html
		static const char* entity_s2c_packet_class_sig = "net/minecraft/class_2684";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/s2c/play/EntityPositionS2CPacket.html
		static const char* entity_position_s2c_packet_class_sig = "net/minecraft/class_2777";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/s2c/play/EntityMoveS2CPacket.html
		static const char* entity_move_s2c_packet_class_sig = "net/minecraft/class_2777";
		// https://maven.fabricmc.net/docs/yarn-1.21.10+build.1/net/minecraft/network/packet/s2c/play/EntityTeleportS2CPacket.html
		static const char* entity_teleport_s2c_packet_class_sig = "net/minecraft/class_2777";

	}
};

#endif // MAPPINGS_HPP