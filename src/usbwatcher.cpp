#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>

#ifdef _WIN32
#	include <windows.h>
#elif defined(__APPLE__)

#else
#	include <unistd.h>
#endif

#define CONFIG_FILE "usbwatcher.conf"
#define DELAY 1000

#define CFG_INTERVAL "interval:"
#define CFG_EXEC "execute:"
#define CFG_ALLOW "allow:"

using namespace std;

struct mount {
#ifdef _WIN32
	string name;
	int type;

	bool operator==(const mount& m) {
		return name == m.name && type == m.type;
	}
#elif defined(__APPLE__)

#else
	string device;
	string destination;
	string fstype;
	string options;
	int dump;
	int pass;

	bool operator==(const mount& m) {
		return device == m.device && destination == m.destination
				&& fstype == m.fstype && options == m.options
				&& dump == m.dump && pass == m.pass;
	}
#endif

	bool operator!=(const mount& m) {
		return !this->operator==(m);
	}
};

static vector<mount> previous;
static vector<string> commands;
static vector<string> whitelist;
static int interval = DELAY;
static bool first = false;

void iterate();
void sleep();
void compare(vector<mount>&);
void trigger(string);
void load_config();

int main(int argc, char* argv[]) {
	load_config();

	while (true) {
		sleep();
		iterate();
	}
}

void load_config() {
	ifstream config(CONFIG_FILE);
	string s;

	while (getline(config, s)) {
		if (s.substr(0, strlen(CFG_EXEC)) == CFG_EXEC) {
			string file = s.substr(strlen(CFG_EXEC));

			commands.push_back(file);
		} else if (s.substr(0, strlen(CFG_INTERVAL)) == CFG_INTERVAL) {
			string i = s.substr(strlen(CFG_INTERVAL));

			interval = atoi(i.c_str());
		} else if (s.substr(0, strlen(CFG_ALLOW)) == CFG_ALLOW) {
			string entry = s.substr(strlen(CFG_ALLOW));

			whitelist.push_back(entry);
		}
	}
}

void iterate() {
	vector<mount> vec;

#ifdef _WIN32
	const int buflen = 256;
	char buffer[buflen];

	char *temp = buffer;

	DWORD result = GetLogicalDriveStrings(buflen, buffer);

	if (result == 0) {
		cerr << "Failed to get drives" << endl;
		exit(0);
	}

	while (*temp) {
		int type = GetDriveType(temp);

		switch (type) {
		case DRIVE_UNKNOWN:
		case DRIVE_REMOVABLE:
		case DRIVE_CDROM:
		case DRIVE_RAMDISK: {
			mount m;
			m.name = string(temp);
			m.type = type;

			vec.push_back(m);
			break;
		}
		default:
			break;
		}

		temp += lstrlen(temp) + 1;
	}

#elif defined(__linux__)
	ifstream file("/proc/mounts");

	while (!file.eof()) {
		mount mount;
		file >> mount.device >> mount.destination >> mount.fstype >> mount.options >> mount.dump >> mount.pass;
		if (!mount.device.empty()) {
			vec.push_back(mount);
		}
	}
#endif

	if (!first) {
		previous = vec;
		first = true;
		return;
	}

	compare(vec);
}

void sleep() {
#ifdef _WIN32
	Sleep(interval);
#else
	usleep(interval * 1000);
#endif
}

void compare(vector<mount>& n) {
	for (unsigned int i = 0; i < n.size(); i++) {
		mount& current = n[i];
		bool exists = false;

		for (unsigned int l = 0; l < previous.size(); l++) {
			mount& prev = previous[l];

			if (prev == current) {
				exists = true;
				break;
			}
		}

		if (!exists) {
			trigger(current.name + " was not here before");
		}
	}

	previous = n;

	return;
}

void trigger(string reason) {
	cout << "TRIGGERED: " << reason << ". Exiting..." << endl;

	for (unsigned int i = 0; i < commands.size(); i++) {
		string s = commands[i];

		cout << "Executing " << s << endl;

		system(s.c_str());
	}

	exit(0);
}
