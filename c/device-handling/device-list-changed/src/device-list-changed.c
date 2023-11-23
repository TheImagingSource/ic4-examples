
#include <stdio.h>
#include <stdlib.h>

#include <ic4/C_ic4.h>

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

void device_list_changed_handler(struct IC4_DEVICE_ENUM* enumerator, void* user_ptr)
{
	ic4_devenum_update_device_list(enumerator);
	int new_device_count = ic4_devenum_get_device_count(enumerator);

	printf("Device list has changed!\n");
	printf("Found %d devices\n", new_device_count);
	printf("\n");
}

int main()
{
	ic4_init_library(NULL);

	struct IC4_DEVICE_ENUM* enumerator = NULL;
	if (!ic4_devenum_create(&enumerator))
	{
		printf("Failed to create device enumerator\n");
		goto print_error;
	}

	if (!ic4_devenum_event_add_device_list_changed(enumerator, device_list_changed_handler, NULL, NULL))
	{
		printf("Failed to register device-list-changed event handler\n");
		goto print_error;
	}

	if (!ic4_devenum_update_device_list(enumerator))
	{
		printf("Failed to update device list\n");
		goto print_error;
	}

	int initial_device_count = ic4_devenum_get_device_count(enumerator);

	printf("Press ENTER to exit program\n");
	printf("%d devices connected initially.\n", initial_device_count);

	(void)getchar();

	// This is technically not necessary, since the DeviceEnum object is destroyed at the end of this function.
	if (!ic4_devenum_event_remove_device_list_changed(enumerator, device_list_changed_handler, NULL))
	{
		printf("Failed to unregister device-list-changed event handler\n");
		goto print_error;
	}

	goto cleanup;
print_error:
	print_last_error();
cleanup:
	ic4_devenum_unref(enumerator);
	ic4_exit_library();
	return 0;
}