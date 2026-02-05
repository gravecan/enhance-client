#pragma once

#include <sdk/includes.h>

namespace sdk
{
	class entity_client
	{
	private:
		jobject entity;

	public:
		entity_client(jobject entity);
		~entity_client();

		jobject get_entity();
		jobject get_bounding_box();
		void set_bounding_box(jobject box);
		bool is_same_object(jobject other);
		double get_x();
		double get_y();
		double get_z();
		float get_yaw();
		float get_pitch();
		void set_yaw(float yaw);
		void set_pitch(float pitch);
		bool is_on_ground();
		double get_fall_distance();
		jobject get_velocity();
};
}

