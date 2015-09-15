#include <iostream>

#ifdef _WIN32
#	include <windows.h>
#elif defined(__linux__)
#   include <fstream>
#	include <unistd.h>
#endif

#define DELAY 1000

using namespace std;

struct mount {
	string device;
	string destination;
	string fstype;
	string options;
	int dump;
	int pass;
};

void iterate();
void sleep();

int main(int argc, char* argv[]) {

	for (;;) {
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

		if (type == DRIVE_UNKNOWN) {
			printf("%s : Unknown Drive type\n", temp);
		} else if (type == DRIVE_REMOVABLE) {
			printf("%s : Removable Drive\n", temp);
		} else if (type == DRIVE_CDROM) {
			printf("%s : CD-Rom/DVD-Rom\n", temp);
		} else if (type == DRIVE_RAMDISK) {
			printf("%s : Ram Drive\n", temp);
		} else {
			cout << "Skipping drive " << temp << endl;
		}

		temp += lstrlen(temp) + 1;
	}

#elif defined(__linux__)
	ifstream mountInfo("/proc/mounts");

	while (!mountInfo.eof()) {
		mount mount;
		mountInfo >> mount.device >> mount.destination >> mount.fstype >> mount.options >> mount.dump >> mount.pass;
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
