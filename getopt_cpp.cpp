/*
 * usage : ./a.out -o /cluster_swdl/ -m manifest.xml -z clusterinstaller.zip 
*/
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string>

void parseCommandLine(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	parseCommandLine(argc, argv);
	return 0;
}

void parseCommandLine(int argc, char *argv[]) {
	int option;
	unsigned int number = 0;
	while((option = getopt(argc, argv, "o:m:z:b:")) != -1){ //get option from the getopt() method
		switch(option){
			case 'o':
				printf("O: Given File: %s\n", optarg);
				break;
			case 'm':
				printf("M: Given File: %s\n", optarg);
				break;
			case 'z':
				printf("Z: Given File: %s\n", optarg);
				break;
			case 'b':
			case 's':
				printf("Z: Given size: %s\n", optarg);
				number = std::stoi(optarg); 
				printf("S: Give int size %d\n", number);
				break;
			case '?':
			case 'h':	
			default:
				printf("H:usage: ./clusterinstaller -o OUTPATH -m MANIFEST_FILE -z UPDATE_ARCHIVE, e.g. ,\n");
				printf("  ./clusterinstaller -o /cluster_swdl/ -m manifest.xml -z clusterinstaller.zip\n");
				break;
		      }
   	}
}
