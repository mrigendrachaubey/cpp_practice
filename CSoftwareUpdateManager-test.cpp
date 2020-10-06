/******************************************************************************
 * Project        BU-Cockpit
 * (c) copyright  2020
 * Company        Harman/Becker Automotive Systems GmbH
 *                All rights reserved
 * Secrecy Level  STRICTLY CONFIDENTIAL
 ******************************************************************************/

/**
 * @file          CSoftwareUpdateManager.cpp
 * @ingroup       Functions dealing with software update management
 * @author        Ravi Shankar Gupta
 * @brief         coordinates with all components for software update
 */

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <CManifestParser.hpp>
#include <CSoftwareInstallerCluster.hpp>
#include <CSoftwareUpdateManager.hpp>
#include <v1/com/harman/swdl/SoftwareInstallerServerImpl.hpp>
#include <CSoftwareInstallerIOC.hpp>
#include <CSwdlMd5Verification.hpp>
#include <CManifestParser.hpp>     
#include <algorithm>
#include <llog.h>



namespace harman{
namespace cockpit{
namespace platform{
namespace clusterInstaller{

CSoftwareUpdateManager::CSoftwareUpdateManager():mActiveBootSlot(BOOT_SLOT_A),
                                                mBootMode(NORMALMODE),
                                                mInstallerStubImpl(nullptr),
                                                mBootControlClient(nullptr)
{
    LLOG_debug1("CSoftwareUpdateManager Constructor");
    socPartList.push_back(XML_NODE_CLUSTERFS);
	//push back ioc part list bolo, app
	iocPartList.push_back(XML_NODE_IOC_BOLO);
	iocPartList.push_back(XML_NODE_IOC_APP);
}

CSoftwareUpdateManager::~CSoftwareUpdateManager()
{
}

/**
 * @brief  : Method to get Singleton instance of class
 * @param  : void
 * @return : void
 */
CSoftwareUpdateManager& CSoftwareUpdateManager::getInstance()
{
    static CSoftwareUpdateManager onlyInstance;
    return onlyInstance;
}

/**
 * @brief  : Method to store the stub instance
 * @param  : instance: stub instance
 * @return : void
 */
void CSoftwareUpdateManager::registerInstallerStubImpl(::v1::com::harman::swdl::SoftwareInstallerServerImpl* instance)
{
    if (instance != nullptr)
    {
        LLOG_info("Setting mInstallerStubImpl");
        mInstallerStubImpl = instance;
    }
	else
	{
		LLOG_error("Unable to get instance ");
	}
}

void CSoftwareUpdateManager::setUpdateArchive(std::string UpdateArchive)
{
     mUpdateArchive = UpdateArchive;
}

std::string CSoftwareUpdateManager::getUpdateArchive(void)
{
      return mUpdateArchive;
}

void CSoftwareUpdateManager::setOutPath(std::string OutPath)
{
      mOutPath = OutPath;
}

std::string CSoftwareUpdateManager::getOutPath(void)
{
      return mOutPath;
}

void CSoftwareUpdateManager::setManifestFile(std::string ManifestFile)
{
      mManifestFile = ManifestFile;
}

std::string CSoftwareUpdateManager::getManifestFile(void) 
{
      return mManifestFile;
}

void CSoftwareUpdateManager::setMaxBlockSize(std::string MaxBlockSize)
{
      mMaxBlockSize = std::stoi(MaxBlockSize);
}

uint64_t CSoftwareUpdateManager::getMaxBlockSize(void)
{
      return mMaxBlockSize;
}

/**
 * @brief  : Method to store command line options
 * @param  : argc, argv
 * @return : void
 */
void CSoftwareUpdateManager::parseCommandLine(int argc, char *argv[])
{
	int option;
	while((option = getopt(argc, argv, "o:m:z:b:")) != -1)
	{
		switch(option)
		{
			case 'o':
			{
				setOutPath(optarg);
				break;
			}
			case 'm':
			{
				setManifestFile(optarg);
				break;
			}
			case 'z':
			{
				setUpdateArchive(optarg);
				break;
			}
			case 'b':
			{
				setMaxBlockSize(optarg);
				break;
			}
			default:
			{
				LLOG_info("usage: ./clusterinstaller -o OUTPATH -m MANIFEST_FILE -z UPDATE_ARCHIVE -b BLOCK_SIZE_BYTES");
				LLOG_info("e.g. ./clusterinstaller -o /cluster_swdl/ -m manifest.xml -z clusterinstaller.zip -b 1048576 ");
				break;
			}
		}
	}
}

/**
 * @brief  : Initializes services of software update
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::init(int argc, char *argv[])
{
    std::string packageName;

	parseCommandLine(argc, argv);

	::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tErrorInfo errorCode_;
    errorCode_.setErrorCode(::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eErrorCode::EERRORCODE_NO_ERROR);
    errorCode_.setErrorString("no error");
    notifyInstallationStatus(packageName, errorCode_,::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState::ESWDLSTATE_IDLE,
        ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification::ESWDLNOTIFICATION_NOTIFY_NONE);
}

/**
 * @brief  : main function of software update
 * @param  : void
 * @return : void
 */
int32_t CSoftwareUpdateManager::SoftwareUpdateMain(v0::harman::cockpit::platform::Rebootmpd::BootControlClientImpl* bootControlClient, int argc, char *argv[])
{
    LLOG_info("CSoftwareUpdateManager::SoftwareUpdateMain");
    mBootControlClient = bootControlClient;
    init(argc, argv);
    return 0;
}

/*
 * verify md5 and send response.
 */
bool CSoftwareUpdateManager::NotifyInstallation(const ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tPackageDetails& packageDetails)
{
    CSwdlMd5Verification Md5obj;
    bool retVal = false;
    std::string packageName = packageDetails.mPackageName;
    std::string zipFile = getUpdateArchive();//clusterinstaller.zip
    LLOG_info( "NotifyInstallation: %s zipFile %s ", packageName.c_str(), zipFile.c_str());
    std::size_t pos_slash = zipFile.rfind("/");
    std::size_t pos_dot = zipFile.find(".");
    ++pos_slash;
    std::string downloadFile = zipFile.substr(pos_slash, pos_dot - pos_slash);//clusterinstaller
    downloadFile += "/";
    downloadFile += getManifestFile();//clusterinstaller/swupdate.xml
    LLOG_info("download file = %s", downloadFile.c_str());
    std::string outFile = getOutPath();
    outFile += getManifestFile(); ///cluster_swdl/swupdate.xml
    
    retVal = DownloadFileFromZip(zipFile, downloadFile, outFile); //clusterinstaller.zip, clusterinstaller/swupdate.xml, cluster_swdl/swupdate.xml
	
    if (retVal == true)
    {
        ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tErrorInfo errorCode_;
        errorCode_.setErrorCode(::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eErrorCode::EERRORCODE_NO_ERROR);
        errorCode_.setErrorString("no error");
        notifyInstallationStatus(packageName, errorCode_,::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState::ESWDLSTATE_VALIDATION_IN_PROGRESS,
            ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification::ESWDLNOTIFICATION_NOTIFY_NONE);
    }
    else
    {
        ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tErrorInfo errorCode_;
        errorCode_.setErrorCode(::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eErrorCode::EERRORCODE_PACKAGE_DOWNLOAD_FAILED);
        errorCode_.setErrorString("Download error");
        notifyInstallationStatus(packageName, errorCode_,::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState::ESWDLSTATE_VALIDATION_FAILED,
            ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification::ESWDLNOTIFICATION_NOTIFY_NONE);
		/*return immediately */
		return retVal;
    }
    CManifestParser::getInstance().parse(outFile); //parse outFile cluster_swdl/swupdate.xml

    retVal = Md5obj.VerifyAll(zipFile);
    LLOG_info("VerifyAll returned %d", static_cast<S32_T>(retVal));
    std::cout << "VerifyAll ret = " << retVal << std::endl;
    if (retVal == true)
    {
        ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tErrorInfo errorCode_;
        errorCode_.setErrorCode(::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eErrorCode::EERRORCODE_NO_ERROR);
        errorCode_.setErrorString("no error");
        notifyInstallationStatus(packageName, errorCode_, ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState::ESWDLSTATE_VALIDATION_COMPLETE,
            ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification::ESWDLNOTIFICATION_NOTIFY_VALIDATION_COMPLETE);
    }
    else
    {
        ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tErrorInfo errorCode_;
        errorCode_.setErrorCode(::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eErrorCode::EERRORCODE_PACKAGE_VALIDATION_FAILED);
        errorCode_.setErrorString("validation error");
        notifyInstallationStatus(packageName, errorCode_, ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState::ESWDLSTATE_VALIDATION_FAILED,
            ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification::ESWDLNOTIFICATION_NOTIFY_INSTALLATION_CANCELLED);
		return retVal;
    }

	return retVal;
    LLOG_info("Notify Installation Complete");
}

/**
 * @brief  : Method to start software update of cluster
 * @param  : packageName: cluster or ioc, eRequestInstall: start, stop, pause
 * @return : true if success else false
 */
bool CSoftwareUpdateManager::StartInstallation(const std::string packageName, const ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eRequestInstall request)
{
    bool retVal = false;
	/*check for soc or ioc*/
	/*
	if(packageName == XML_NODE_SYS_DOM)
	{
		LLOG_info("cluster to update\n");
	}
	else if(packageName == XML_NODE_IOC_DOM)
	{
	             //rebootToRecoveryMode if ioc update to happen
                if(isApplicationMode() == APPLICATION_NORMAL_MODE)
                {
                    LLOG_info( "update request is in recovery mode, so booting to recvery mode %d",isApplicationMode());
                    rebootToRecoveryMode();
                    sleep(60*TIME_1_SEC);//should reboot in 60 sec
                    LLOG_info( "Do manual reboot to go bolo mode %d",isApplicationMode());
                }

	}
*/
    if(request == ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eRequestInstall::EREQUESTINSTALL_START_INSTALLATION)
    {
        LLOG_info("Request received for starting installation of %s", packageName.c_str());
        retVal = CSoftwareUpdateManager::StartUpdateJob(packageName, getOutPath());

    }
    return retVal;
}

bool CSoftwareUpdateManager::isApplicationMode()
{
	//some reboot manager logic will give us appmode
	return true;
}

/**
 * @brief  : This method updates one part
 * @param  : partName: Name of part, type: part type cluster or ioc, sourcePath: path of part
 * @return : true if success else false
 */
bool CSoftwareUpdateManager::StartUpdateJob(const std::string packageName, const std::string sourcePath)
{
    bool retVal = false;
    std::list <std::string>::iterator it;
	std::list<std::string> partList;
	if(isApplicationMode() == true)
	{
		partList = socPartList;
		mNumComponents = partList.size();
	}
	else //if recovery mode
	{
		partList = iocPartList;
		mNumComponents = partList.size();
		
	}

    ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tErrorInfo errorCode_;
    errorCode_.setErrorCode(::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eErrorCode::EERRORCODE_NO_ERROR);
    errorCode_.setErrorString("no error");

    retVal = notifyInstallationStatus(packageName, errorCode_,::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState::ESWDLSTATE_INSTALLATION_IN_PROGRESS,
        ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification::ESWDLNOTIFICATION_NOTIFY_NONE);

	std::list<std::string>::iterator it;

	for(it = partList.begin(); it != partList.end(); it++)
	{
		std::cout << "starting update of: " << it->c_str() << std::endl;
		if(it->c_str() == XML_NODE_SYS_DOM)
		{
			std::cout << "Updating" << std::endl;
			mCurrentPartName = it->c_str();
			ClusterUpdate clusterFsupdate(mCurrentPartName);
			mJobBase = dynamic_cast<Interface*>(&clusterFsupdate);
			createWorkerThread();
			CreateProgressThread();
			mComponentsUpdated++;
		}
		else if(it->c_str() == XML_NODE_IOC_DOM)
		{
			mCurrentPartName = it->c_str();
			ClusterUpdate recoveryFsupdate(mCurrentPartName);
			mJobBase = dynamic_cast<Interface*>(&recoveryFsupdate);
			createWorkerThread();
			CreateProgressThread();	
			mComponentsUpdated++;
		}
		else
		{
			std::cout << "No match found" << std::endl;
		}
		mJobBase = nullptr;
	}

    notifyInstallationStatus(packageName, errorCode_,::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState::ESWDLSTATE_INSTALLATION_COMPLETE,
        ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification::ESWDLNOTIFICATION_NOTIFY_NONE);

    return retVal;
}

/**
 * @brief  : Creates Worker thread to update the part
 * @param  : partName: Name of part, type: part type cluster or ioc, sourcePath: path of part
 * @return : true if success else false
 */
bool CSoftwareUpdateManager::CreateUpdateWorkerThread()
{
		pthread_t threadid;
		pthread_create(&threadid, NULL, updateThread, this);
}

/**
 * @brief  : Creates Worker thread to update progress
 * @param  : void
 * @return : true if success else false
 */
bool CSoftwareUpdateManager::CreateProgressThread()
{
    int err;
    bool retVal = true;
    pthread_t threadId;
    err = pthread_create(&threadId, NULL, progressThread, this);
    if (err != 0)
    {
        LLOG_error("error in creating Action thread error = %d", err);
        retVal = false;
    }
    err = pthread_join(threadId, NULL);
    if (err != 0)
    {
        LLOG_error("error in joining Action thread error = %d", err);
        retVal = false;
    }
    return retVal;
}

/**
 * @brief  : Thread function to update progress
 * @param  : ptr to object of class CSoftwareUpdateManager
 * @return : true if success else false
 */
void* CSoftwareUpdateManager::progressThread(void* ptr)
{
    LLOG_info("CSoftwareUpdateManager::progressThread");
    ((CSoftwareUpdateManager*)ptr)->runProgress();
    return NULL;
}


/**
 * @brief  : Thread function to update part
 * @param  : ptr to object of class CSoftwareUpdateManager
 * @return : true if success else false
 */
void* CSoftwareUpdateManager::updateThread(void* ptr)
{
    LLOG_info("CSoftwareUpdateManager::updateThread");
    pthread_detach(pthread_self());
    ((CSoftwareUpdateManager*)ptr)->run();
    return NULL;
}

/**
 * @brief  : function to update part
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::run()
{
    LLOG_info(" CSoftwareUpdateManager::run");
    if((mJobBase->preInstall() == true) && (mJobBase->install() == true) && (mJobBase->postInstall() == true))
     {
		LLOG_info("Part %s update successful", mCurrentPartName.c_str());
     }
	else
	{
		LLOG_info("Part %s update failed");
	}
}

/**
 * @brief  : function to update progress
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::runProgress()
{
    LLOG_info("CSoftwareUpdateManager::runProgress");
    U8_T progress = 0U;
    U8_T lastProgress = 0U;
    /* LDRA_INSPECTED 70 D: lastProgress is used */
    while(progress < MAX_PROGRESS_PERCENT)
    {
        progress = static_cast<U8_T>(mJobBase->getProgress());
        if(progress != lastProgress)
        {
            notifyInstallationProgress(progress, mCurrentPartName);
            lastProgress = progress;
            LLOG_info("Progress sent = %d", static_cast<U32_T>(lastProgress));
            LLOG_info("Progress percent received = ", static_cast<U32_T>(lastProgress));
        }
        usleep(TIME_US_10000);
    }

}

/**
 * @brief  : Stores the current state of installation
 * @param  : state: state to be updated
 * @return : void
 */
void CSoftwareUpdateManager::setClusterInstallState(const std::string& state) const
{
    /* LDRA_INSPECTED 1 D: state to be implemented */
}

/**
 * @brief  : Method to get current installation state
 * @param  : void
 * @return : current installation state
 */
const std::string CSoftwareUpdateManager::CSoftwareUpdateManager::getClusterInstallState() const
{
    return "";
}

/**
 * @brief  : Method to get if parts update is pending
 * @param  : void
 * @return : true if parts update pending else false
 */
bool CSoftwareUpdateManager::isPartsUpdatePending() const
{
    return true;
}

/**
 * @brief  : Clean the images at startup if software update is not pending
 * @param  : dir_full_path: path of directory to be cleaned
 * @return : true if success else false
 */
bool CSoftwareUpdateManager::cleanupCacheOnStartup(std::string dir_full_path) const
{
    /* LDRA_INSPECTED 1 D: dir_full_path to be implemented */
    return true;
}

/**
 * @brief  : Remove the updated part from persistent list
 * @param  : part: name of part
 * @return : void
 */
void CSoftwareUpdateManager::removePartUpdated(const std::string& part) const
{
    /* LDRA_INSPECTED 1 D: part to be implemented */
}

/**
 * @brief  : Requests reboot manager to reboot system in normal mode
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::rebootToNormalMode()
{
    if(mBootControlClient != nullptr)
    {
        mBootControlClient->sendRebootToNormalModeRequest();
        sleep(TIME_1_SEC);
        mBootControlClient->sendRebootRequest();
    }
	else
	{
		LLOG_error("Error: rebootToNormalMode failed` ");
	}
}

/**
 * @brief  : Requests reboot manager to reboot in recovery mode
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::rebootToRecoveryMode()
{
    if(mBootControlClient != nullptr)
    {
        mBootControlClient->sendRebootToRecoveryModeRequest();
        sleep(TIME_1_SEC);
        mBootControlClient->sendRebootRequest();
    }
	else
	{
		LLOG_error("Error: rebootToRecoveryMode failed` ");
	}
}

/**
 * @brief  : Sends update progress to IVI
 * @param  : partPercent: percentage completition of current part getting updated, partName: name of part
 * @return : void
 */
void CSoftwareUpdateManager::notifyInstallationProgress(const U8_T partPercent, const std::string& partName)
{
    /* LDRA_INSPECTED 57 S: installProgress is used to set installprogress struct */
    ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tInstallationProgress installProgress;
    installProgress.setOverallProgress(partPercent);
    installProgress.setCurrModuleProgress(partPercent);
    installProgress.setPartName(partName);

    if(mInstallerStubImpl != nullptr)
    {
        mInstallerStubImpl->set_installProgress(installProgress);
    }
	else
	{
		LLOG_error("Error: notifyInstallationProgress failed` ");
	}
}


/**
 * @brief  : Sends current installation state to IVI
 * @param  : void
 * @return : void
 */
bool CSoftwareUpdateManager::notifyInstallationStatus(const std::string packageName, const ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tErrorInfo &errorInfo, const ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlState currentState, const ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::eSwdlNotification notification)
{
    /* LDRA_INSPECTED 57 S: installProgress is used to set status_ struct */
    ::v1::com::harman::swdl::SoftwareInstallerTypesSAPILow::tStatus status;
    bool retVal = false;
    status.setPackageName(packageName);
    status.setCurrentState(currentState);
    status.setErrorInfo(errorInfo);
    status.setNotification(notification);

    LLOG_info("notifyInstallationStatus: %s current state = %d", packageName.c_str(), currentState);
    if(mInstallerStubImpl != nullptr)
    {
        mInstallerStubImpl->set_SoftwareUpdateState(status);
		retVal = true;
    }
	else
	{
		LLOG_error("Error: mInstallerStubImpl null");
	}

	return retVal;
}

/**
 * @brief  : Set Current active boot slot
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::setActiveBootSlot(const BOOT_SLOT_e bootSlot)
{
    mActiveBootSlot = bootSlot;
}

/**
 * @brief  : Get update boot slot
 * @param  : void
 * @return : BOOT_SLOT_e
 */
BOOT_SLOT_e CSoftwareUpdateManager::getUpdateBootSlot() const
{
    BOOT_SLOT_e updateBootSlot;
    if(mActiveBootSlot == BOOT_SLOT_A)
    {
        updateBootSlot = BOOT_SLOT_B;
    }
    else if(mActiveBootSlot == BOOT_SLOT_B)
    {
        updateBootSlot = BOOT_SLOT_A;
    }
    else
    {
        updateBootSlot = BOOT_SLOT_NONE;
    }
    return updateBootSlot;
}

/**
 * @brief  : Download file from zip
 * @param  : zipFile: zip file name with path, file: name of file inside zip, outPath: path at which file is to be copied
 * @return : true if success else false
 */
bool CSoftwareUpdateManager::DownloadFileFromZip(const std::string& zipFile, const std::string& file, const std::string& outPath) const
{
    bool retVal = false;
    if(access(zipFile.c_str(), F_OK) != 0)
    {
        LLOG_error("Zip file %s not found", zipFile.c_str());
    }
    else
    {
        int err;
        zip* zp = zip_open(zipFile.c_str(), 0, &err);
        if(zp != NULL)
        {
            struct zip_stat st;
            std::string downloadFile = file;
            zip_stat(zp, downloadFile.c_str(), 0, &st);
            LLOG_info("File %s size = %u", file.c_str(), st.size);
            zip_file* zfp = zip_fopen(zp, downloadFile.c_str(), 0);
            if(zfp != NULL)
            {
                std::string outFile = outPath;
                char buff[ZIP_BLOCK_SIZE];
                std::ofstream outf(outFile.c_str(), std::ios::binary);
                int64_t readVal = 0;
                U32_T numLoops = st.size/ZIP_BLOCK_SIZE;
                if(st.size % ZIP_BLOCK_SIZE)
                {
                    numLoops++;
                }
                for(U32_T loop = 0U; loop < numLoops; loop++)
                {
                    readVal = zip_fread(zfp, buff, ZIP_BLOCK_SIZE);
                    outf.write(buff, readVal);
                }
                outf.close();
				zip_fclose(zfp);
				zip_close(zp);
				retVal = true;
            }
			else
			{
				LLOG_error("file open failed %s", file.c_str());
			}
        }
		else
		{
			LLOG_error("failed to open zip error = %d", err);
		}

    }
    return retVal;
}

/**
 * @brief  : Get integer value for ASCII bootinfo byte
 * @param  :
arg : ASCII bootinfo value
val : On success, integer value will be stored here
 * @return : RESULT_FAILURE : If failed
RESULT_SUCCESS : If value successfully converted to int
 */
int8_t CSoftwareUpdateManager::getVal(const char arg, U8_T &val)
{
    int8_t ret = static_cast<int8_t>(RESULT_FAILURE);

    if ((arg >= '0') && (arg <= '9'))
    {
        val = static_cast<int8_t>(arg - '0');
        ret = static_cast<int8_t>(RESULT_SUCCESS);
    }
    else if ((arg >= 'A') && (arg <= 'F'))
    {
        val = static_cast<int8_t>(arg - 'A' + 10);
        ret = static_cast<int8_t>(RESULT_SUCCESS);
    }
    else if ((arg >= 'a') && (arg <= 'f'))
    {
        val = static_cast<int8_t>(arg - 'a' + 10);
        ret = static_cast<int8_t>(RESULT_SUCCESS);
    }
    else
    {
        LLOG_info("Invalid value received for bootinfo");
    }

    return ret;
}
/**
 * @brief  : Method to update bootInfo received from reboot manager
 * @param  : string: bootInfo
 * @return : void
 * TODO: use correct logging functions.
 */
int8_t CSoftwareUpdateManager::updateBootInfo(const std::string& bootInfo)
{
    int8_t ret = RESULT_FAILURE;
    U8_T clusterBootMode=0U;/* 0 => hyp A, 1 => hyp B, 2 => hyp A recovery, 3 =>hyp B recovery */
    const char clusterBootSlot = bootInfo[CLUSTERBOOTINDEX];

    if (ret != getVal(clusterBootSlot, clusterBootMode))
    {
        LLOG_error("Failed to get Active slot from bootinfo");
    }
    else
    {
        LLOG_info("Cluster BootSlot: %d", static_cast<S32_T>(clusterBootMode));
        if(clusterBootMode == CLUSTERFS_B || clusterBootMode == RECOVERYFS_B)
        {
            mActiveBootSlot = BOOT_SLOT_B;
        }
        else
        {
            mActiveBootSlot = BOOT_SLOT_A;
        }
        if(clusterBootMode == CLUSTERFS_A || clusterBootMode == CLUSTERFS_B)
        {
            mBootMode = NORMALMODE;
        }
        else
        {
            mBootMode = RECOVERYMODE;
        }
		
		LLOG_info("mActiveBootSlot = %d", mActiveBootSlot);
		LLOG_info( "mBootMode = %d ", mBootMode);
		ret = RESULT_SUCCESS;
    }

    return ret;
}

/**
 * @brief  : Interface method to ask reboot manager to set cluster Active partition
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::setClusterActivePartition()
{
    if(mBootControlClient != nullptr)
    {
        ::v0::harman::cockpit::platform::Rebootmpd::BootControlSAPILow::BootSlot_e newBootSlot =
        static_cast<::v0::harman::cockpit::platform::Rebootmpd::BootControlSAPILow::BootSlot_e>(getUpdateBootSlot());
        mBootControlClient->setClusterActiveSlot(newBootSlot);
    }
	else
	{
		LLOG_error("Error: mBootControlClient null ");
	}
}

/**
 * @brief  : Interface method to ask reboot manager to set IVI Active partition
 * @param  : void
 * @return : void
 */
void CSoftwareUpdateManager::setIviActivePartition()
{
    //To Do: This method is to revert boot slot of IVI in failed update of ioc or peripheral
    //Mechanism to retrieve old shall be implemented, below implementation is temporary
    if(mBootControlClient != nullptr)
    {
        ::v0::harman::cockpit::platform::Rebootmpd::BootControlSAPILow::BootSlot_e oldIviBootSlot =
        ::v0::harman::cockpit::platform::Rebootmpd::BootControlSAPILow::BootSlot_e::BOOTSLOT_E_SLOT_A;
        mBootControlClient->setIviBootPartitionRequest(oldIviBootSlot);
    }
	else
	{
		LLOG_error("Error: mBootControlClient null ");
	}
}

}//namespace clusterInstaller
}//namespace platform
}//namespace cockpit
}//namespace harman


