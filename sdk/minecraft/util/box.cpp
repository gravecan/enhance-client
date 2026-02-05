#include <enhance/enhance.h>
#include "box.h"

sdk::box_client::box_client(jobject box)
{
	this->box = box;
}

sdk::box_client::~box_client()
{
}

double sdk::box_client::get_min_x()
{
	auto env = enhance::instance->get_env();
	if (!env || !box)
		return 0.0;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class)
		return 0.0;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_min_x_name, sdk::mappings::box_min_x_sig);
	if (!fid)
	{
		env->DeleteLocalRef(box_class);
		return 0.0;
	}

	jdouble ret = env->GetDoubleField(box, fid);
	env->DeleteLocalRef(box_class);

	return ret;
}

double sdk::box_client::get_max_x()
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return 0.0;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return 0.0;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_max_x_name, sdk::mappings::box_max_x_sig);
	if (!fid)
	{
		env->DeleteLocalRef(box_class);
		return 0.0;
	}

	jdouble ret = env->GetDoubleField(box, fid);
	env->DeleteLocalRef(box_class);

	return ret;
}

double sdk::box_client::get_min_y()
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return 0.0;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return 0.0;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_min_y_name, sdk::mappings::box_min_y_sig);
	if (!fid)
	{
		env->DeleteLocalRef(box_class);
		return 0.0;
	}

	jdouble ret = env->GetDoubleField(box, fid);
	env->DeleteLocalRef(box_class);

	return ret;
}

double sdk::box_client::get_max_y()
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return 0.0;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return 0.0;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_max_y_name, sdk::mappings::box_max_y_sig);
	if (!fid)
	{
		env->DeleteLocalRef(box_class);
		return 0.0;
	}

	jdouble ret = env->GetDoubleField(box, fid);
	env->DeleteLocalRef(box_class);

	return ret;
}

double sdk::box_client::get_min_z()
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return 0.0;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return 0.0;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_min_z_name, sdk::mappings::box_min_z_sig);
	if (!fid)
	{
		env->DeleteLocalRef(box_class);
		return 0.0;
	}

	jdouble ret = env->GetDoubleField(box, fid);
	env->DeleteLocalRef(box_class);

	return ret;
}

double sdk::box_client::get_max_z()
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return 0.0;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return 0.0;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_max_z_name, sdk::mappings::box_max_z_sig);
	if (!fid)
	{
		env->DeleteLocalRef(box_class);
		return 0.0;
	}

	jdouble ret = env->GetDoubleField(box, fid);
	env->DeleteLocalRef(box_class);

	return ret;
}

void sdk::box_client::set_min_x(double value)
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_min_x_name, sdk::mappings::box_min_x_sig);
	if (fid)
	{
		env->SetDoubleField(box, fid, value);
	}

	env->DeleteLocalRef(box_class);
}

void sdk::box_client::set_max_x(double value)
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_max_x_name, sdk::mappings::box_max_x_sig);
	if (fid)
	{
		env->SetDoubleField(box, fid, value);
	}

	env->DeleteLocalRef(box_class);
}

void sdk::box_client::set_min_y(double value)
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_min_y_name, sdk::mappings::box_min_y_sig);
	if (fid)
	{
		env->SetDoubleField(box, fid, value);
	}

	env->DeleteLocalRef(box_class);
}

void sdk::box_client::set_max_y(double value)
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_max_y_name, sdk::mappings::box_max_y_sig);
	if (fid)
	{
		env->SetDoubleField(box, fid, value);
	}

	env->DeleteLocalRef(box_class);
}

void sdk::box_client::set_min_z(double value)
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_min_z_name, sdk::mappings::box_min_z_sig);
	if (fid)
	{
		env->SetDoubleField(box, fid, value);
	}

	env->DeleteLocalRef(box_class);
}

void sdk::box_client::set_max_z(double value)
{
	auto env = enhance::instance->get_env();
	if (!env || !box) return;

	jclass box_class = env->GetObjectClass(box);
	if (!box_class) return;

	jfieldID fid = env->GetFieldID(box_class, sdk::mappings::box_max_z_name, sdk::mappings::box_max_z_sig);
	if (fid)
	{
		env->SetDoubleField(box, fid, value);
	}

	env->DeleteLocalRef(box_class);
}

