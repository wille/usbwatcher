#include <iostream>
#include <vector>

#ifdef _WIN32
#	include <windows.h>
#elif defined(__APPLE__)

#else
#   include <fstream>
#	include <unistd.h>
#endif

#define DELAY 1000

using namespace std;

struct mount {
#ifdef _WIN32
	string name;
	int type;
#elif defined(__APPLE__)

#else
	string device;
	string destination;
	string fstype;
	string options;
	int dump;
	int pass;
#endif
};

static vector<mount> previous;

void iterate();
void sleep();

int main(int argc, char* argv[]) {
	while (true) {
		sleep();
		iterate();
	}
}

void iterate() {
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
		case DRIVE_RAMDISK:
			mount m;
			m.name = string(temp);
			m.type = type;
			break;
		default:
			cout << "Skipping drive " << temp << endl;
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
}

void sleep() {
#ifdef _WIN32
	Sleep(DELAY);
#else
	usleep(DELAY * 1000);
#endif
}
