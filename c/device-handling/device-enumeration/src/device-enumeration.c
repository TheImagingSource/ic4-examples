
#include <stdio.h>
#include <stdlib.h>

#include <ic4/C_ic4.h>

void print_device_info(const struct IC4_DEVICE_INFO* device_info)
{
	printf("Model: %s ", ic4_devinfo_get_model_name(device_info));
	printf("Serial: %s", ic4_devinfo_get_serial(device_info));
	printf("Version: %s", ic4_devinfo_get_version(device_info));
}

void print_last_error()
{
	enum IC4_ERROR code;
	size_t buffer_size = 0;

	if (!ic4_get_last_error(&code, NULL, &buffer_size))
	{
		printf("Failed to query error message length\n");
		return;
	}

	char* message = malloc(buffer_size);

	if (!ic4_get_last_error(&code, message, &buffer_size))
	{
		printf("Failed to query error message\n");
	}
	else
	{
		printf("Error (code %d): %s\n", code, message);
	}

	free(message);
}

void print_device_list()
{
	printf("Enumerating all attached video capture devices in a single list...\n");

	struct IC4_DEVICE_ENUM* enumerator = NULL;
	if (!ic4_devenum_create(&enumerator))
	{
		printf("Failed to create device enumerator");
		goto print_error;
	}

	if (!ic4_devenum_update_device_list(enumerator))
	{
		printf("Failed to update device list");
		goto print_error;
	}

	int count = ic4_devenum_get_device_count(enumerator);
	if (count == 0)
	{
		printf("No devices found\n");
		goto cleanup;
	}
	
	printf("Found %d devices:\n", count);
	for (int i = 0; i < count; ++i)
	{
		struct IC4_DEVICE_INFO* info = NULL;
		if (!ic4_devenum_get_devinfo(enumerator, i, &info))
		{
			printf("Failed to query device info for index %d\n", i);
			print_last_error();
			continue;
		}

		printf("\t");
		print_device_info(info);
		printf("\n");

		ic4_devinfo_unref(info);
	}

	printf("\n");
	goto cleanup;

print_error:
	print_last_error();

cleanup:
	ic4_devenum_unref(enumerator);
}

const char* tltype_to_string(enum IC4_TL_TYPE val)
{
	switch (val)
	{
	case IC4_TLTYPE_GIGEVISION: return "GigEVision";
	case IC4_TLTYPE_USB3VISION: return "USB3Vision";
	case IC4_TLTYPE_UNKNOWN: return "Unknown";
	default:
		return "";
	}
}

void print_interface_device_tree()
{
	printf("Enumerating video capture devices by interface...\n");

	struct IC4_DEVICE_ENUM* enumerator = NULL;
	if (!ic4_devenum_create(&enumerator))
	{
		printf("Failed to create device enumerator");
		goto print_error;
	}

	if (!ic4_devenum_update_interface_list(enumerator))
	{
		printf("Failed to update device list");
		goto print_error;
	}

	int itf_count = ic4_devenum_get_interface_count(enumerator);
	if (itf_count == 0)
	{
		printf("No interfaces found\n");
		goto cleanup;
	}

	for (int i = 0; i < itf_count; ++i)
	{
		struct IC4_INTERFACE* itf = NULL;
		if (!ic4_devenum_get_devitf(enumerator, i, &itf))
		{
			printf("Failed to query interface info for index %d\n", i);
			print_last_error();
			continue;
		}

		printf("Interface: %s\n", ic4_devitf_get_display_name(itf));
		printf("\tProvided by %s [TLType: %s]\n", ic4_devitf_get_tl_name(itf), tltype_to_string(ic4_devitf_get_tl_type(itf)));

		if (!ic4_devitf_update_device_list(itf))
		{
			printf("Failed to update interface device list");
			print_last_error();
			ic4_devitf_unref(itf);
			continue;
		}

		int dev_count = ic4_devitf_get_device_count(itf);
		if (dev_count == 0)
		{
			printf("\tNo devices found\n");
			ic4_devitf_unref(itf);
			continue;
		}

		printf("\tFound %d devices:\n", dev_count);
		for (int j = 0; j < dev_count; ++j)
		{
			struct IC4_DEVICE_INFO* info = NULL;
			if (!ic4_devitf_get_devinfo(itf, j, &info))
			{
				printf("Failed to query device info for index %d\n", j);
				print_last_error();
				continue;
			}

			printf("\t\t");
			print_device_info(info);
			printf("\n");

			ic4_devinfo_unref(info);
		}

		ic4_devitf_unref(itf);
	}

	printf("\n");
	goto cleanup;

print_error:
	print_last_error();

cleanup:
	ic4_devenum_unref(enumerator);
}

int main()
{
	ic4_init_library(NULL);

	print_device_list();
	print_interface_device_tree();

	ic4_exit_library(),

	return 0;
}