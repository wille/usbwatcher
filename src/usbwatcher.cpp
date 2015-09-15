#include <iostream>
#include <vector>

#ifdef _WIN32
#	include <windows.h>
#elif defined(__APPLE__)

#else
#	include <fstream>
#	include <unistd.h>
#endif

#define DELAY 1000

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

void iterate();
void sleep();
void compare(vector<mount>&);
void trigger(string);

int main(int argc, char* argv[]) {
	while (true) {
		sleep();
		iterate();
		cout << endl;
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
			cout << mount.fstype << " device \"" << mount.device << "\", mounted on \"" << mount.destination << "\". Options: " << mount.options << ". Dump:" << mount.dump << " Pass:" << mount.pass << endl;
		}
	}
#endif

	compare(vec);
}

void sleep() {
#ifdef _WIN32
	Sleep(DELAY);
#else
	usleep(DELAY * 1000);
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
	cout << "TRIGGERED: " << reason << ". Exiting...";
	exit(0);
}
