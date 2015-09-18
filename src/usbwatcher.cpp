#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <sstream>

#ifdef _WIN32
#	include <windows.h>
#elif defined(__APPLE__)

#else
#	include "../../quickd/daemon.h"
#	include <unistd.h>
#endif

#define CONFIG_FILE "usbwatcher.conf"
#define DELAY 1000
#define KEYFILE_BITS 4096

#define CFG_INTERVAL "interval:"
#define CFG_EXEC "execute:"
#define CFG_ALLOW "allow:"
#define CFG_KEYFILE "keyfile:"

#define OPT_LIST "--list"
#define OPT_LIST_SHORT "-l"
#define OPT_HELP "--help"
#define OPT_HELP_SHORT "-h"
#define OPT_DAEMON "--daemon"
#define OPT_GENKEY "--genkey"

using namespace std;

static vector<string> whitelist;

struct mount {
#ifdef _WIN32
	string name;
	string destination;
	int type;
	string serial;

	bool operator==(const mount& m) {
		return name == m.name && type == m.type && serial == m.serial;
	}
#elif defined(__APPLE__)

#else
	string name;
	string destination;
	string fstype;
	string options;
	int dump;
	int pass;

	bool operator==(const mount& m) {
		return name == m.name && destination == m.destination
				&& fstype == m.fstype && options == m.options
				&& dump == m.dump && pass == m.pass;
	}
#endif

	bool operator!=(const mount& m) {
		return !this->operator==(m);
	}


	bool is_whitelisted() {
		for (unsigned int i = 0; i < whitelist.size(); i++) {
			string w = whitelist[i];

#ifdef _WIN32
			if (name == w || serial == w) {
				return true;
			}
#elif defined(__APPLE__)

#else
			if (name == w || destination == w) {
				return true;
			}
#endif
		}

		return false;
	}
};

static vector<mount> previous;
static vector<string> commands;
static vector<string> keyfiles;
static int interval = DELAY;
static bool first = false;

void iterate(bool print = false);
void sleep();
void compare(vector<mount>&);
void trigger(string);
void load_config();
bool check_keyfile(const mount&);
string trim(string);

void print_info() {
	cout << "usbwatcher, Built on: " << __DATE__ << " " << __TIME__ << endl;
}

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		char* opt = argv[i];

		if (!strcmp(opt, OPT_LIST_SHORT) || !strcmp(opt, OPT_LIST)) {
			print_info();
			iterate(true);
		} else if (!strcmp(opt, OPT_HELP_SHORT) || !strcmp(opt, OPT_HELP)) {
			print_info();
			exit(0);
		} else if (!strcmp(opt, OPT_DAEMON)) {
#ifdef __linux__
			deploy_daemon();
#else
			cerr << "Running as daemon is only supported on Linux!" << endl;
#endif
		} else if (!strcmp(opt, OPT_GENKEY)) {
			char* path = "keyfile";

			if (i + 1 < argc) {
				path = argv[++i];
			}

			ofstream file(path, ios::out | ios::binary);

			srand(time(NULL));
			for (int i = 0; i < KEYFILE_BITS / 8; i++) {
				char c = rand();
				file << c;
			}

			file.close();

			cout << "Generated " << KEYFILE_BITS << " bit keyfile to " << path << endl;

			exit(0);
		}
	}

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
			string file = trim(s.substr(strlen(CFG_EXEC)));

			commands.push_back(file);
		} else if (s.substr(0, strlen(CFG_INTERVAL)) == CFG_INTERVAL) {
			string i = trim(s.substr(strlen(CFG_INTERVAL)));

			interval = atoi(i.c_str());
		} else if (s.substr(0, strlen(CFG_ALLOW)) == CFG_ALLOW) {
			string entry = trim(s.substr(strlen(CFG_ALLOW)));

			whitelist.push_back(entry);
		} else if (s.substr(0, strlen(CFG_KEYFILE)) == CFG_KEYFILE) {
			string entry = trim(s.substr(strlen(CFG_KEYFILE)));

			ostringstream o;

			ifstream in(entry.c_str(), ios::binary);
			o << in.rdbuf();

			string data = o.str();

			keyfiles.push_back(data);
		}
	}
}

void iterate(bool print) {
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

	if (print) {
		cout << "MOUNT\t\tSERIAL" << endl;
		cout << "=====\t\t======" << endl;
	}

	while (*temp) {
		int type = GetDriveType(temp);

		switch (type) {
		case DRIVE_UNKNOWN:
		case DRIVE_REMOVABLE:
		case DRIVE_CDROM:
		case DRIVE_RAMDISK: {
			DWORD serial = 0;
			GetVolumeInformation(temp, NULL, 0, &serial, NULL, NULL, NULL, 0);


			char text_serial[16];
			sprintf(text_serial, "%04x-%04x", HIWORD(serial), LOWORD(serial));

			mount m;
			m.name = string(temp);
			m.destination = m.name;
			m.type = type;
			m.serial = string(text_serial);

			if (print) {
				cout << temp << "\t\t" << text_serial << endl;
				break;
			}

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
		file >> mount.name >> mount.destination >> mount.fstype >> mount.options >> mount.dump >> mount.pass;
		if (!mount.name.empty()) {
			vec.push_back(mount);
		}
	}
#endif

	if (print) {
		return;
	}

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

		if (!exists && !current.is_whitelisted() && !check_keyfile(current)) {
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

bool check_keyfile(const mount& m) {
	ostringstream o;

	string path = m.destination + "/keyfile";
	ifstream in(path.c_str(), ios::binary);

	if (in.good()) {
		o << in.rdbuf();

		string data = o.str();

		if (keyfiles.size() > 0) {
			for (unsigned int i = 0; i < keyfiles.size(); i++) {
				string key = keyfiles[i];

				if (key == data) {
					return true;
				}
			}
		}
	}

	return false;
}

string trim(string s) {
	const char* TRIM = "\t\r\n ";
	int first = s.find_first_not_of(TRIM);
	int last = s.find_last_not_of(TRIM);

	if (s.length() == 0 || first == -1 || last == -1) {
		return s;
	}

	s = s.substr(first, last + 1);

	return s;
}
