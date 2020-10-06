#include <iostream>
using namespace std;
const std::string MANIFEST_FILE = "swupdate.xml";
const std::string outPath = "/cluster_swdl/";


int main(int argc, char* argv[])
{
    std::string outFile = outPath;
    outFile += MANIFEST_FILE;
    std::cout << "" << outPath.c_str() << std::endl;
    return 0;
}
