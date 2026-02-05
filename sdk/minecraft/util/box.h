#pragma once

#include <sdk/includes.h>

namespace sdk
{
	class box_client
	{
	private:
		jobject box;

	public:
		box_client(jobject box);
		~box_client();

		double get_min_x();
		double get_max_x();
		double get_min_y();
		double get_max_y();
		double get_min_z();
		double get_max_z();

		void set_min_x(double value);
		void set_max_x(double value);
		void set_min_y(double value);
		void set_max_y(double value);
		void set_min_z(double value);
		void set_max_z(double value);
	};
}

