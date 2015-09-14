#include <iostream>

#ifdef _WIN32
#	include <windows.h>
#endif

using namespace std;

void iterate();

int main(int argc, char* argv[]) {
	iterate();
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

#endif
}
