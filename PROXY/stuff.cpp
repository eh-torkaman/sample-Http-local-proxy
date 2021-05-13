#include <vector>
#include <memory>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <iostream>
#include <dirent.h>
#include <list>
#include <iterator>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>

using namespace std;

uint64_t timeSinceEpochMillisec()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t timeSinceEpochMillisecOfT(std::time_t tt)
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::from_time_t (tt).time_since_epoch()).count();
}

 
 bool fileExistInCache (const std::string name) {
  struct stat buffer;   
  return (stat (("./CACHE/"+name).c_str(), &buffer) == 0); 
}


 bool fileInUseInCache1 (const std::string& name) {
    ifstream f(("./CACHE/"+name).c_str());
    return f.good();
}

 bool fileInUseInCache2 (const std::string& name) {
    if (FILE *file = fopen(("./CACHE/"+name).c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}


std::vector<std::string> GetDirectoryFiles(const std::string &dir)
{
	std::vector<std::string> files;
	std::shared_ptr<DIR> directory_ptr(opendir(dir.c_str()), [](DIR *dir) { dir &&closedir(dir); });
	struct dirent *dirent_ptr;
	if (!directory_ptr)
	{
		std::cout << "Error opening : " << std::strerror(errno) << dir << std::endl;
		return files;
	}

	while ((dirent_ptr = readdir(directory_ptr.get())) != nullptr)
	{
		files.push_back(std::string(dirent_ptr->d_name));
	}
	return files;
}


void fs(int n, string inStr, bool addEndline = true)
{
	string rs;
	switch (n)
	{
	case 1:
		rs = "\x1B[32m" + string(inStr) + "\033[0m";
		break;
	case 2:
		rs = ("\x1B[31m" + inStr + "\033[0m");
		break;
	case 3:
		rs = ("\x1B[33m" + inStr + "\033[0m");
		break;
	case 4:
		rs = ("\x1B[34m" + inStr + "\033[0m");
		break;
	case 5:
		rs = ("\x1B[35m" + inStr + "\033[0m");
		break;
	case 6:
		rs = ("\x1B[36m" + inStr + "\033[0m");
		break;
	case 7:
		rs = ("\x1B[36m" + inStr + "\033[0m");
		break;
	case 8:
		rs = ("\x1B[36m" + inStr + "\033[0m");
		break;
	case 9:
		rs = ("\x1B[37m" + inStr + "\033[0m");
		break;

	case 10:
		rs = ("\x1B[93m" + inStr + "\033[0m");
		break;
	case 11:
		rs = ("\033[3;42;30m" + inStr + "\033[0m");
		break;
	case 12:
		rs = ("\033[3;43;30m" + inStr + "\033[0m");
		break;
	case 13:
		rs = ("\033[3;44;30m" + inStr + "\033[0m");
		break;
	case 14:
		rs = ("\033[3;104;30m" + inStr + "\033[0m");
		break;
	case 15:
		rs = ("\033[3;100;30m" + inStr + "\033[0m");
		break;
	case 16:
		rs = ("\033[3;47;35m" + inStr + "\033[0m");
		break;
	case 17:
		rs = ("\033[2;47;35m" + inStr + "\033[0m");
		break;
	case 18:
		rs = ("\033[1;47;35m" + inStr + "\033[0m");
		break;
	default:
		rs = inStr;
	}
	if (addEndline)
		cout << endl
			 << rs << endl;
	else
		cout << rs;
}
