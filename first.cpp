#include <CL/cl.h>
#include <vector>
#include <string>
#include <iostream>
#include <iosfwd>

struct Device
{
	std::string m_type;
	std::string m_name;
	std::string m_vendor;
	std::string m_version;
	std::string m_profile;
	std::string m_driver_version;
};

std::ostream& operator << (std::ostream& os, const Device& d)
{
	os << "\t\tType:       " << d.m_type << std::endl;
	os << "\t\tName:       " << d.m_name << std::endl;
	os << "\t\tVendor:     " << d.m_vendor << std::endl;
	os << "\t\tVersion:    " << d.m_version << std::endl;
	os << "\t\tProfile:    " << d.m_profile << std::endl;
	os << "\t\tDriver ver: " << d.m_driver_version << std::endl;
	return os;
}

struct Platform
{
	std::string m_name;
	std::string m_vendor;
	std::string m_version;
	std::vector<Device> m_devices;
};

std::ostream& operator << (std::ostream& os, const Platform& p)
{
	os << "\tName:      " << p.m_name << std::endl;
	os << "\tVendor:    " << p.m_vendor << std::endl;
	os << "\tVersion:   " << p.m_version << std::endl;
	os << "\tHas " << p.m_devices.size() << " devices" << std::endl;
	int i = 0;
	for (auto d : p.m_devices)
	{
		os << "\t Device[" << i++ << "]:" << std::endl;
		os << d;
	}
	return os;
}

bool GetDeviceType(cl_device_id device_id, std::string& res)
{
	cl_int err;
	cl_device_type deviceType;
	err = clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, 0);
	if (err == CL_SUCCESS)
	{
		res += (deviceType & CL_DEVICE_TYPE_DEFAULT) ? "DEFAULT " : "";
		res += (deviceType & CL_DEVICE_TYPE_CPU) ? "CPU " : "";
		res += (deviceType & CL_DEVICE_TYPE_GPU) ? "GPU " : "";
		res += (deviceType & CL_DEVICE_TYPE_ACCELERATOR) ? "ACCELERATOR " : "";
		res += (deviceType & CL_DEVICE_TYPE_CUSTOM) ? "CUSTOM " : "";
		return true;
	}
	return false;
}

bool GetDeviceInfo(cl_device_id device_id, cl_device_info info_id, std::string& res)
{
	if (info_id == CL_DEVICE_TYPE)
	{
		return GetDeviceType(device_id, res);
	}
	cl_int err;
	cl_uint size;
	err = clGetDeviceInfo(device_id, info_id, 0, nullptr, &size);
	if (err == CL_SUCCESS)
	{
		res.resize(size);
		err = clGetDeviceInfo(device_id, info_id, size, &res[0], nullptr);
		if (err == CL_SUCCESS)
			return true;
	}
	return false;
}

bool GetInfoAboutDevice(cl_device_id device_id, Device& device)
{
	bool res = GetDeviceInfo(device_id, CL_DEVICE_NAME, device.m_name);
	res &= GetDeviceInfo(device_id, CL_DEVICE_VENDOR, device.m_vendor);
	res &= GetDeviceInfo(device_id, CL_DEVICE_VERSION, device.m_version);
	res &= GetDeviceInfo(device_id, CL_DEVICE_PROFILE, device.m_profile);
	res &= GetDeviceInfo(device_id, CL_DRIVER_VERSION, device.m_driver_version);
	res &= GetDeviceInfo(device_id, CL_DEVICE_TYPE, device.m_type);
	return res;
}

void CollectDevices(cl_platform_id platform_id, std::vector<Device>& devices)
{
	std::vector<cl_device_id> device_ids;
	cl_uint size;
	cl_int err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 0, nullptr, &size);
	if (err != CL_SUCCESS)
		return;
	device_ids.resize(size);
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, size, &device_ids[0], nullptr);
	for (auto d: device_ids)
	{
		Device cur_device;
		if (GetInfoAboutDevice(d, cur_device))
			devices.push_back(cur_device);
	}
}

bool GetPlatformInfo(cl_platform_id platform_id, cl_platform_info info_id, std::string& res)
{
	cl_int err;
	cl_uint size;
	err = clGetPlatformInfo(platform_id, info_id, 0, nullptr, &size);
	if (err == CL_SUCCESS)
	{
		res.resize(size);
		err = clGetPlatformInfo(platform_id, info_id, size, &res[0], nullptr);
		if (err == CL_SUCCESS)
			return true;
	}
	return false;
}

bool GetInfoAboutPlatform(cl_platform_id platform_id, Platform& platform)
{
	bool res = GetPlatformInfo(platform_id, CL_PLATFORM_NAME, platform.m_name);
	res &= GetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, platform.m_vendor);
	res &= GetPlatformInfo(platform_id, CL_PLATFORM_VERSION, platform.m_version);
	CollectDevices(platform_id, platform.m_devices);
	return res;
}

void CollectPlatforms(std::vector<Platform>& platforms)
{
	cl_int err;
	cl_uint num_platforms;
	err = clGetPlatformIDs(0, nullptr, &num_platforms);
	if (err != CL_SUCCESS)
		return;
	std::vector<cl_platform_id> platrorm_ids;
	platrorm_ids.resize(num_platforms);
	err = clGetPlatformIDs(num_platforms, &platrorm_ids[0], nullptr);
	if (err != CL_SUCCESS)
		return;
	for (auto p : platrorm_ids)
	{
		Platform cur_platform;
		if (GetInfoAboutPlatform(p, cur_platform))
			platforms.push_back(cur_platform);
	}
}

int main()
{
	std::vector<Platform> platforms;
	CollectPlatforms(platforms);
	std::cout <<"Enumerated " << platforms.size() << " platforms on system." << std::endl;
	int i = 0;
	for (auto p : platforms)
	{
		std::cout << " Platform[" << i++ << "]:" << std::endl;
		std::cout << p;
	}
	return 0;
}