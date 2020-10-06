bool CSoftwareInstallerCluster::install()
{
/*
	uint32_t bytesUpdated = 0;
	std::list<std::string> partitionList = CManifestParser::getInstance().getPartitionList(XML_NODE_SYS_DOM, mPartName);
	std::string partition;
	uint8_t data[BLOCK_SIZE];
	if(mUpdateBootSlot == BOOT_SLOT_A)
	{
		partition = *(partitionList.begin());
	}
	else if(mUpdateBootSlot == BOOT_SLOT_B)
	{
		std::list<std::string>::iterator it = partitionList.begin();
		++it;
		partition = *it;
	}

	std::string imageNameWithPath = outPath+mImageName;
	int32_t inFd = open(imageNameWithPath.c_str(), O_RDONLY);
	int32_t outFd = open(partition.c_str(), O_RDWR);
	
	while(bytesUpdated <= mImageSize)
	{
		uint32_t bytesToRead = BLOCK_SIZE;
		if((mImageSize - bytesUpdated) < BLOCK_SIZE)
		{
			bytesToRead = mImageSize - bytesUpdated;
			read(inFd, data, bytesToRead);
			write(outFd, data, bytesToRead);
			bytesUpdated += bytesToRead;
			updateProgress(bytesUpdated);
		}
	};
*/
    //Open the ZIP archive
    int err = 0, fd;
    zip *z = zip_open("clusterinstaller.zip", 0, &err);

    //Search for the file of given name
    const char *name = "rootfs.ext4";//argv[1];
    struct zip_stat st;
    zip_stat_init(&st);
    zip_stat(z, name, 0, &st);

    printf("Name: [%s], ", st.name);
    printf("Size: [%lu], ", st.size);
    printf("mtime: [%u]\n", (unsigned int)st.mtime);
    //Alloc memory for its uncompressed contents
    char *contents = new char[st.size];

    //Read the compressed file
    zip_file *f = zip_fopen(z, name, 0);//name = rootfs.ext4
    fd = open("/dev/mmcblk0p28s", O_RDWR, 0644);
    zip_fread(f, contents, st.size);
    write(fd, contents, st.size);
    close(fd);
    zip_fclose(f);

    //And close the archive
    zip_close(z);

    //Do something with the contents
    //delete allocated memory
    delete[] contents;

	return true;
}

