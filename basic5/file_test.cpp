#include <stdio.h>
#include <string>
#include <list>
#include <dirent.h>
#include <map>
#include <dlfcn.h>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <vector>

using namespace std;

const int32_t SEAT_NO = 1;
const std::string DOWNLOAD_PACKAGE_PATH   = "cache/";
const std::string PARTS_TO_UPDATE = "cache/parts.txt";
const std::string MENIFEST_FILE_PATH  = "cache/swupdate.xml";
const std::string XML_NODE_CLUSTERFS("CLUSTERFS");
const std::string XML_NODE_SYS_DOM("SOC_DOM");
const std::string XML_NODE_IOC_DOM("IOC_DOM");
const std::string XML_NODE_IOC_BOLO("IOC_BOLO");
const std::string XML_NODE_IOC_APP("IOC_APP");
const std::string ELEMENT_TYPE("TYPE");
const std::string ELEMENT_PART_TYPE("PART_TYPE");
const std::string ELEMENT_IMAGE_NAME("IMAGE_NAME");
const std::string ELEMENT_PARTITION("PARTITION");
const std::string ELEMENT_VERSION_CHECK("VERSION_CHECK");
const std::string ELEMENT_MD5("MD5");
const std::string ELEMENT_AUTHENTICITY_CHECK("AUTHENTICITY_CHECK");
const std::string ELEMENT_UPDATE_MODE("UPDATE_MODE");

struct Node
{
    std::string authenticityCheck;
    std::list<std::string> imageNameList;
    std::list<std::string> partitionList;
    std::string updateMode;
    std::string versionCheck;
    std::list<std::string> Md5SumList;
    std::string partName;
    std::string domainName;
};

vector <string> mPartsInOrder;
std::map <std::string, Node> NodeMap;
typedef map<string, list<string> > partMetadata_t; /* map<partName,values > */
typedef map<string, partMetadata_t> partInfo_t; /* map< partName, partMetaData > */

void getPartNames(list<string>& partName);
void ParsePart(const std::string& DomainName, const std::string& PartName, xmlNodePtr child);
void handleSemicolon(std::string str, std::list<std::string>& lists);
bool parse(const std::string& file);
void print_element_names(const xmlNodePtr cur_node); 

void getPartNames(list<string>& partName);
void removePartUpdated(const std::string& part);
bool isPartsUpdatePending();
bool checkPartToBeUpdated(std::string part);
bool cleanupCacheOnStartup(std::string dir_full_path);

void handleSemicolon(std::string str, std::list<std::string>& lists)
{
    std::size_t k = 0U;
    for(std::size_t i = 0U; i < str.length(); i++)
    {
        if(str.at(i) == ';')
        {
            lists.push_back(str.substr(k,(i-k)));
            k = i+1U;
        }
    }
    lists.push_back(str.substr(k));
}


void ParsePart(const std::string& DomainName, const std::string& PartName, xmlNodePtr child)
{
    Node partNode;
    while(child != NULL)
    {
        /*LDRA_INSPECTED 95 S: typecast ok*/
        std::string childNode = reinterpret_cast<const char*>(child->name);
        if(childNode.find(ELEMENT_AUTHENTICITY_CHECK) != std::string::npos)
        {
            /*LDRA_INSPECTED 70 D: struct member inserted into map*/
            /*LDRA_INSPECTED 440 S: typecast ok*/
            partNode.authenticityCheck = std::string(reinterpret_cast<const char*>(xmlNodeGetContent(child)));
        }
        else if(childNode.find(ELEMENT_IMAGE_NAME) != std::string::npos)
        {
            std::list<std::string>	imageList;
            /*LDRA_INSPECTED 440 S: typecast ok*/
            std::string imageNames = std::string(reinterpret_cast<const char*>(xmlNodeGetContent(child)));
            printf("ImagesNames: %s\n", imageNames.c_str());
            handleSemicolon(imageNames, imageList);
            /*LDRA_INSPECTED 70 D: struct member inserted into map*/
            /*LDRA_INSPECTED 440 S: typecast ok*/
            partNode.imageNameList = imageList;
        }
        else if(childNode.find(ELEMENT_PARTITION) != std::string::npos)
        {
            std::list<std::string>	partitionList;
            /*LDRA_INSPECTED 440 S: typecast ok*/
            std::string partitionNames = std::string(reinterpret_cast<const char*>(xmlNodeGetContent(child)));
            handleSemicolon(partitionNames, partitionList);
            /*LDRA_INSPECTED 70 D: struct member inserted into map*/
            partNode.partitionList = partitionList;
        }
        else if(childNode.find(ELEMENT_VERSION_CHECK) != std::string::npos)
        {
            /*LDRA_INSPECTED 70 D: struct member inserted into map*/
            /*LDRA_INSPECTED 440 S: typecast ok*/
            partNode.versionCheck = std::string(reinterpret_cast<const char*>(xmlNodeGetContent(child)));
        }
        else if(childNode.find(ELEMENT_MD5) != std::string::npos)
        {
            std::list<std::string>	Md5SumList;
            /*LDRA_INSPECTED 440 S: typecast ok*/
            std::string md5Sums = std::string(reinterpret_cast<const char*>(xmlNodeGetContent(child)));
            handleSemicolon(md5Sums, Md5SumList);
            /*LDRA_INSPECTED 70 D: struct member inserted into map*/
            partNode.Md5SumList = Md5SumList;
        }
        else if(childNode.find(ELEMENT_UPDATE_MODE) != std::string::npos)
        {
            /*LDRA_INSPECTED 70 D: struct member inserted into map*/
            /*LDRA_INSPECTED 440 S: typecast ok*/
            partNode.updateMode = std::string(reinterpret_cast<const char*>(xmlNodeGetContent(child)));
        }
        else
        {
            //unsupported node
        }

        child = child->next;
    }
    if((DomainName.empty() == false) && (PartName.empty() == false))
    {
        std::string key = DomainName + PartName;
        partNode.partName = PartName;
        partNode.domainName = DomainName;

        NodeMap.insert(std::pair<std::string, Node>(key, partNode));
    }
}

bool parse(const std::string& file)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    bool retVal = true;

    NodeMap.clear();

    /*parse the file and get the DOM */
    doc = xmlReadFile(file.c_str(), NULL, 0);
    if (doc == NULL)
    {
        printf("XML Parse error: %s\n", file.c_str());
        retVal = false;
    }
    if(retVal == true)
    {
        /*Get the root element node */
        root_element = xmlDocGetRootElement(doc);
        while (root_element != nullptr)
        {
            print_element_names(root_element);
            root_element = root_element->next;
        }
        xmlFreeDoc(doc);       // free document
        xmlCleanupParser();    // Free globals
    }
    return retVal;
}

void print_element_names(const xmlNodePtr cur_node)
{
    if (cur_node->type == XML_ELEMENT_NODE)
    {
        xmlNodePtr cur = cur_node;
        printf("Node Name: %s\n", cur->name);
        cur = cur->xmlChildrenNode;

        while (cur != NULL)
        {
            if (cur->type == XML_ELEMENT_NODE)
            {
                
                std::string Domain = reinterpret_cast<const char*>(cur->name);
                
                std::string DomainName;
                if(Domain.find(XML_NODE_SYS_DOM) != std::string::npos)
                {
                    DomainName = XML_NODE_SYS_DOM.c_str();
                }
                else if(Domain.find(XML_NODE_IOC_DOM) != std::string::npos)
                {
                    DomainName = XML_NODE_IOC_DOM.c_str();
                }
                else
                {
                    //unsupported node
                }
                printf("Domain: %s\n", Domain.c_str());
                xmlNodePtr child = cur->xmlChildrenNode;
                while(child != NULL)
                {
                    std::string Part = reinterpret_cast<const char*>(child->name);
                    std::string PartName;
                    if(Part.find(XML_NODE_CLUSTERFS) != std::string::npos)
                    {
                        PartName = XML_NODE_CLUSTERFS.c_str();
                    }
                    else if(Part.find(XML_NODE_IOC_BOLO) != std::string::npos)
                    {
                        PartName = XML_NODE_IOC_BOLO.c_str();
                    }
                    else if(Part.find(XML_NODE_IOC_APP) != std::string::npos)
                    {
                        PartName = XML_NODE_IOC_APP.c_str();
                    }

                    if(PartName.empty() == false)
                    {
                        printf("PartName: %s\n", PartName.c_str());
						mPartsInOrder.push_back(std::string(reinterpret_cast<const char*>(PartName.c_str())));
                        ParsePart(DomainName, PartName, child->xmlChildrenNode);
                    }
                    child = child->next;
                }
            }
            cur = cur->next;
        }
		
    }
}
/*void print_element_names(xmlNodePtr cur_node)
{
    if (cur_node->type == XML_ELEMENT_NODE)
    {
        printf( "node type: Element, name: %s value %s\n",cur_node->name, xmlNodeGetContent(cur_node));
        partMetadata_t metaData;
        xmlNodePtr cur = cur_node;
        cur = cur->xmlChildrenNode;

        while (cur != NULL)
        {
            if (cur->type == XML_ELEMENT_NODE)
            {
                printf( "node type: Element, name: %s value %s\n",cur->name, xmlNodeGetContent(cur));
                 string curNodeContent = std::string(reinterpret_cast<const char*>(xmlNodeGetContent(cur)));
                 list<string> lists;
                handleSemicolon(curNodeContent,lists);
                metaData.insert(
                        std::pair<string, list<string> >(
                                std::string(
                                        reinterpret_cast<const char*>(cur->name)),lists));
            }
            cur = cur->next;
        }
        mPartsInOrder.push_back(std::string(reinterpret_cast<const char*>(cur_node->name)));
    }
}*/

void getPartNames(list<string>& partName)
{
    list<string> listOfparts;
    vector<string>::iterator it = mPartsInOrder.begin();
    for (; it != mPartsInOrder.end(); it++)
    {
        printf("part: %s\n",(*it).c_str());
        listOfparts.push_back(*it);
    }

    partName = listOfparts;
}



bool cleanupCacheOnStartup(std::string dir_full_path)
{
    bool retval = true;
    DIR* dirp = opendir(dir_full_path.c_str());
    if(NULL != dirp)
    {
        struct dirent *dir;
        struct stat st;
        dir = readdir(dirp);
        while(dir != NULL)
        {
            if(0 != strncmp(dir->d_name,".", SEAT_NO))
            {
                const int INVALID_VAL = -1;
                std::string sub_path = dir_full_path + "/" + dir->d_name;
                if( INVALID_VAL != lstat(sub_path.c_str(),&st))
                {
                    if(0 != S_ISREG(st.st_mode))
                    {
                        if (INVALID_VAL == unlink(sub_path.c_str()))
                        {
                            retval = false;
                            printf("Unlink %s error\n", sub_path.c_str());
                        }
                    }
                }
                else
                {
                   retval = false;
                   printf("Lstat %s error\n", sub_path.c_str());
                }
            }
            dir = readdir(dirp);
        }
        closedir(dirp);
    }
    else
    {
        retval = false;
        printf("Opendir %s error\n", dir_full_path.c_str());
    }
	printf("%s removed\n", dir_full_path.c_str());
    return retval;
}

bool isPartsUpdatePending()
{
    /*LDRA_INSPECTED 67 X: no ambiguity */
    bool retVal = true;

    printf("isPartsUpdatePending\n");
    if (access(PARTS_TO_UPDATE.c_str(), F_OK) == 0)
    {
        printf("%s file found\n", PARTS_TO_UPDATE.c_str() );

        std::ifstream file;

        file.open (PARTS_TO_UPDATE.c_str());
        if(file.is_open() == true)
        {
            std::string parts;
            file >> parts;
            file.close();
            printf("B4 :  no of PartsUpdatePending() %ld\n", parts.length());
            printf("B4 :  no of PartsUpdatePending() %s\n", parts.c_str());
            if(parts.empty() == true)
            {
                retVal = false;
            }
        }
        else
        {
            retVal = false;
        }
    }
    else
    {
        printf("%s file not found\n",PARTS_TO_UPDATE.c_str() );
        retVal = false;
    }
    return retVal;
}

void removePartUpdated(const std::string& part)
{

    if (access(PARTS_TO_UPDATE.c_str(), F_OK) == 0)
    {
        printf("%s file found\n", PARTS_TO_UPDATE.c_str());
		std::ifstream inFile;
		inFile.open (PARTS_TO_UPDATE.c_str());
		printf("removePartUpdated %s\n", part.c_str());
        /*LDRA_INSPECTED 70 D: partsToUpdate is used */
		std::string partsToUpdate;
        if(inFile.is_open() == true)
        {
		    inFile >> partsToUpdate;
		    inFile.close();
        }

		printf("parts read from file [%s]\n", partsToUpdate.c_str());
        const size_t found = partsToUpdate.find(part);
		if(found != std::string::npos)
		{
			printf("removePartUpdated %s found\n", part.c_str());
			//Remove the part name and the following semi-colon
			partsToUpdate.erase(found, strlen(part.c_str())+1U); 
			if(partsToUpdate.length() == 0)
			{
				printf("removePartUpdated all parts updated [%s]\n", partsToUpdate.c_str());
				printf("removing the file %s", PARTS_TO_UPDATE.c_str());
				if(remove(PARTS_TO_UPDATE.c_str()) < 0)
                {
                    printf("Failed removing file %s\n", PARTS_TO_UPDATE.c_str());
                }
				if(remove(MENIFEST_FILE_PATH.c_str()) < 0)
                {
                    printf("Failed removing file %s\n", MENIFEST_FILE_PATH.c_str());
                }
			}
			else
			{
				std::ofstream outFile;
				outFile.open (PARTS_TO_UPDATE.c_str(), ios::trunc);
				if(outFile.is_open() == true)
				{
					outFile << partsToUpdate;
					outFile.close();
				}
			}
			printf("removePartUpdated part: %s partsToUpdate =%s\n", part.c_str(), partsToUpdate.c_str());
			sync();
		}
	}
	else
    {
        printf("%s file not found\n",PARTS_TO_UPDATE.c_str());

    } 

}

bool checkPartToBeUpdated(std::string part)
{
    bool retVal = false;
    printf("checkPartToBeUpdated %s\n", part.c_str());

    if (access(PARTS_TO_UPDATE.c_str(), F_OK) == 0)
    {
        printf("%s file found\n", PARTS_TO_UPDATE.c_str() );
        std::ifstream file;
        file.open (PARTS_TO_UPDATE.c_str());

        if(file.is_open() == true)
        {
            std::string partsToUpdate;
            file >> partsToUpdate;//reading from a file
            printf("parts read from file [%s]\n", partsToUpdate.c_str());
            file.close();
            if(partsToUpdate.find(part) != std::string::npos)
            {
                printf("checkPartToBeUpdated %s found\n", part.c_str());
                retVal = true;
            }
            else
            {
                retVal = false;
            }
        }
    }
    else
    {
        printf("%s file not found\n",PARTS_TO_UPDATE.c_str() );
        retVal = false;
    }
    return retVal;
}

int main(int argc, char *argv[])
{
	std::string partName;
	std::string outFile = MENIFEST_FILE_PATH;
	parse(outFile);
	
	std::string partsToUpdate;
	list < string > partsList;
	getPartNames(partsList);
	if( access (PARTS_TO_UPDATE.c_str(), F_OK) == 0)
	{
		printf("file %s is present... \n", PARTS_TO_UPDATE.c_str());
	}
	
	for (list<string>::iterator partNameItr = partsList.begin(); partNameItr != partsList.end(); partNameItr++)
	{
		partName = (*partNameItr);
		printf("\npartNameItr %s \n",partName.c_str());
		partsToUpdate.append(partName);
		partsToUpdate.append(";");
	}
	ofstream file(PARTS_TO_UPDATE.c_str(), ios::trunc);
	if(file.is_open() == true)
	{
		file << partsToUpdate.c_str();
		file.close();
	}
	printf("file %s contents pushed to %s...\n", partsToUpdate.c_str(),PARTS_TO_UPDATE.c_str());
	isPartsUpdatePending();
	checkPartToBeUpdated(XML_NODE_CLUSTERFS);
	removePartUpdated(XML_NODE_CLUSTERFS);
	cleanupCacheOnStartup(DOWNLOAD_PACKAGE_PATH);
	return 0;
}