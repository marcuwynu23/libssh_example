#include <stdio.h>
#include <libssh/libssh.h>

int main()
{
	// Terminal input variables
	char hostname[100];
	char username[100];
	char password[100];
	char command[1000];

	// Get hostname, username, and password from the user
	printf("Enter hostname: ");
	scanf("%s", hostname);
	printf("Enter username: ");
	scanf("%s", username);
	printf("Enter password: ");
	scanf("%s", password);

	// Create a new SSH session
	ssh_session my_ssh_session;
	int verbosity = SSH_LOG_PROTOCOL;

	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL)
	{
		printf("Error creating SSH session\n");
		return 1;
	}

	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, hostname);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT_STR, "22"); // Set your SSH port if it's not the default (22)
	ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

	// Connect to the server
	int rc = ssh_connect(my_ssh_session);
	if (rc != SSH_OK)
	{
		fprintf(stderr, "Error connecting to %s: %s\n", hostname, ssh_get_error(my_ssh_session));
		return 1;
	}

	// Authenticate yourself
	rc = ssh_userauth_password(my_ssh_session, NULL, password);
	if (rc != SSH_AUTH_SUCCESS)
	{
		fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(my_ssh_session));
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
		return 1;
	}

	printf("Successfully connected to SSH server at %s!\n", hostname);

	// Terminal interaction loop
	while (1)
	{
		printf("Enter command to execute on the server (type 'exit' to quit): ");
		scanf(" %[^\n]", command); // Read the whole line including spaces

		if (strcmp(command, "exit") == 0)
		{
			break; // Exit the loop if the user types 'exit'
		}

		// Execute the command on the server
		ssh_channel channel = ssh_channel_new(my_ssh_session);
		if (channel == NULL)
		{
			fprintf(stderr, "Error creating channel: %s\n", ssh_get_error(my_ssh_session));
			continue; // Continue to the next iteration of the loop
		}

		rc = ssh_channel_open_session(channel);
		if (rc != SSH_OK)
		{
			fprintf(stderr, "Error opening channel: %s\n", ssh_get_error(my_ssh_session));
			ssh_channel_free(channel);
			continue; // Continue to the next iteration of the loop
		}

		rc = ssh_channel_request_exec(channel, command);
		if (rc != SSH_OK)
		{
			fprintf(stderr, "Error executing command: %s\n", ssh_get_error(my_ssh_session));
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			continue; // Continue to the next iteration of the loop
		}

		// Read and print the output of the command
		char buffer[256];
		int nbytes;
		while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0)
		{
			fwrite(buffer, 1, nbytes, stdout);
		}

		if (nbytes < 0)
		{
			fprintf(stderr, "Error reading channel: %s\n", ssh_get_error(my_ssh_session));
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			continue; // Continue to the next iteration of the loop
		}

		ssh_channel_send_eof(channel);
		ssh_channel_close(channel);
		ssh_channel_free(channel);
	}

	// Disconnect and cleanup
	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);

	return 0;
}
