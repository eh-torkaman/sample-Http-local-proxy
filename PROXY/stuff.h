#include <string>
#include <vector>
#include <memory>

using namespace std;

bool fileExistInCache (const std::string name);
bool fileInUseInCache2 (const std::string& name) ;
bool fileInUseInCache1 (const std::string& name) ;

uint64_t timeSinceEpochMillisec(); //milliseconds ms = duration_cast< milliseconds >(     system_clock::now().time_since_epoch() );
uint64_t timeSinceEpochMillisecOfT(std::time_t tt);

std::vector<std::string> GetDirectoryFiles(const std::string &dir);

void fs(int n, string inStr, bool addEndline = true);

struct fileAccessData
{
  string fileName;
  uint64_t lastTimeAccessed;
  uint64_t numberOfTimeAccessed;
  uint64_t firstTimeCreated; //time(&time1);   // get current time after time pass.
  uint64_t getData(int chacheStrategy)
  {
    switch (chacheStrategy)
    {
    case 1:
      return firstTimeCreated;
      break;
    case 2:
      return lastTimeAccessed;
      break;

    default:
      return numberOfTimeAccessed;
      break;
    }
  }
};


bool compare_fileAccessData (const fileAccessData& first, const fileAccessData& second);
 

uint64_t timeSinceEpochMillisec(); //milliseconds ms = duration_cast< milliseconds >(     system_clock::now().time_since_epoch() );