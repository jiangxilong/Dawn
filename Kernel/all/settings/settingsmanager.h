#ifndef _SETTINGS_MANAGER_DEF_H_
#define _SETTINGS_MANAGER_DEF_H_

struct settings_manager_entry
{
	char* name;
	char* data;

	struct settings_manager_entry* next;
};

typedef struct settings_manager_entry settingsEntry;

void initializeSettingsManager();
unsigned char settingsExecuteLine(char const* line);
const char* settingsReadValue(char const* name, char const* defaultValue);

#endif //_SETTINGS_MANAGER_DEF_H_