//#pragma warning(disable : 4996)

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Helper.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <dirent.h>
#ifdef __cplusplus
}
#endif

#include <string.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_timer.h>
#include "../../include/defines.h"

#include "ClassLogFile.h"

#include "esp_vfs_fat.h"
#include "../sdmmc_common.h"

static const char* TAG = "HELPER";

using namespace std;

unsigned int systemStatus = 0;

sdmmc_cid_t SDCardCid;
sdmmc_csd_t SDCardCsd;
bool SDCardIsMMC;

// #define DEBUG_DETAIL_ON 

/////////////////////////////////////////////////////////////////////////////////////////////
string getESPHeapInfo(){
	string espInfoResultStr = "";
	char aMsgBuf[80];

	size_t aFreeHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT);

	size_t aFreeSPIHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
	size_t aFreeInternalHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

	size_t aHeapLargestFreeBlockSize = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
	size_t aHeapIntLargestFreeBlockSize = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

	size_t aMinFreeHeapSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
	size_t aMinFreeInternalHeapSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);


	sprintf(aMsgBuf,"Heap Total: %ld", (long) aFreeHeapSize);
	espInfoResultStr += string(aMsgBuf);

	sprintf(aMsgBuf," | SPI Free: %ld", (long) aFreeSPIHeapSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," | SPI Large Block:  %ld", (long) aHeapLargestFreeBlockSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," | SPI Min Free: %ld", (long) aMinFreeHeapSize);
	espInfoResultStr += string(aMsgBuf);

	sprintf(aMsgBuf," | Int Free: %ld", (long) (aFreeInternalHeapSize));
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," | Int Large Block:  %ld", (long) aHeapIntLargestFreeBlockSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," | Int Min Free: %ld", (long) (aMinFreeInternalHeapSize));
	espInfoResultStr += string(aMsgBuf);
	
	return 	espInfoResultStr;
}


size_t getESPHeapSize()
{
   return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}


size_t getInternalESPHeapSize() 
{
	return heap_caps_get_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_INTERNAL);
}


string getSDCardPartitionSize(){
	FATFS *fs;
    uint32_t fre_clust, tot_sect;

    /* Get volume information and free clusters of drive 0 */
    f_getfree("0:", (DWORD *)&fre_clust, &fs);
    tot_sect = ((fs->n_fatent - 2) * fs->csize) /1024 /(1024/SDCardCsd.sector_size);	//corrected by SD Card sector size (usually 512 bytes) and convert to MB

	//ESP_LOGD(TAG, "%d MB total drive space (Sector size [bytes]: %d)", (int)tot_sect, (int)fs->ssize);

	return std::to_string(tot_sect);
}


string getSDCardFreePartitionSpace(){
	FATFS *fs;
    uint32_t fre_clust, fre_sect;
  
    /* Get volume information and free clusters of drive 0 */
    f_getfree("0:", (DWORD *)&fre_clust, &fs);
    fre_sect = (fre_clust * fs->csize) / 1024 /(1024/SDCardCsd.sector_size);	//corrected by SD Card sector size (usually 512 bytes) and convert to MB

    //ESP_LOGD(TAG, "%d MB free drive space (Sector size [bytes]: %d)", (int)fre_sect, (int)fs->ssize);

	return std::to_string(fre_sect);
}


string getSDCardPartitionAllocationSize(){
	FATFS *fs;
    uint32_t fre_clust, allocation_size;
  
    /* Get volume information and free clusters of drive 0 */
    f_getfree("0:", (DWORD *)&fre_clust, &fs);
    allocation_size = fs->ssize;

    //ESP_LOGD(TAG, "SD Card Partition Allocation Size: %d bytes", allocation_size);

	return std::to_string(allocation_size);
}


void SaveSDCardInfo(sdmmc_card_t* card) {
	SDCardCid = card->cid;
    SDCardCsd = card->csd;
	SDCardIsMMC = card->is_mmc;
}


string getSDCardManufacturer(){
	string SDCardManufacturer = SDCardParseManufacturerIDs(SDCardCid.mfg_id);
	//ESP_LOGD(TAG, "SD Card Manufacturer: %s", SDCardManufacturer.c_str());
	
	return (SDCardManufacturer + " (ID: " + std::to_string(SDCardCid.mfg_id) + ")");
}


string getSDCardName(){
	char *SDCardName = SDCardCid.name;
	//ESP_LOGD(TAG, "SD Card Name: %s", SDCardName); 

	return std::string(SDCardName);
}


string getSDCardCapacity(){
	int SDCardCapacity = SDCardCsd.capacity / (1024/SDCardCsd.sector_size) / 1024;  // total sectors * sector size  --> Byte to MB (1024*1024)
	//ESP_LOGD(TAG, "SD Card Capacity: %s", std::to_string(SDCardCapacity).c_str()); 

	return std::to_string(SDCardCapacity);
}


string getSDCardSectorSize(){
	int SDCardSectorSize = SDCardCsd.sector_size;
	//ESP_LOGD(TAG, "SD Card Sector Size: %s bytes", std::to_string(SDCardSectorSize).c_str()); 

	return std::to_string(SDCardSectorSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////

void memCopyGen(uint8_t* _source, uint8_t* _target, int _size)
{
    for (int i = 0; i < _size; ++i)
        *(_target + i) = *(_source + i);
}


std::string FormatFileName(std::string input)
{
#ifdef ISWINDOWS_TRUE
    input.erase(0, 1);
    std::string os = "/";
    std::string ns = "\\";
    FindReplace(input, os, ns);
#endif
    return input;
}


std::size_t file_size(const std::string& file_name) {
    std::ifstream file(file_name.c_str(),std::ios::in | std::ios::binary);
    if (!file) return 0;
    file.seekg (0, std::ios::end);
    return static_cast<std::size_t>(file.tellg());
}


void FindReplace(std::string& line, std::string& oldString, std::string& newString) {
    const size_t oldSize = oldString.length();

    // do nothing if line is shorter than the string to find
    if (oldSize > line.length()) return;

    const size_t newSize = newString.length();
    for (size_t pos = 0; ; pos += newSize) {
        // Locate the substring to replace
        pos = line.find(oldString, pos);
        if (pos == std::string::npos) return;
        if (oldSize == newSize) {
            // if they're same size, use std::string::replace
            line.replace(pos, oldSize, newString);
        }
        else {
            // if not same size, replace by erasing and inserting
            line.erase(pos, oldSize);
            line.insert(pos, newString);
        }
    }
}


/**
 * Create a folder and its parent folders as needed
 */
bool MakeDir(std::string path)
{
	std::string parent;

	LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Creating folder " + path + "...");

	bool bSuccess = false;
    int nRC = ::mkdir( path.c_str(), 0775 );
    if( nRC == -1 )
    {
        switch( errno ) {
            case ENOENT:
                //parent didn't exist, try to create it
				parent = path.substr(0, path.find_last_of('/'));
        		LogFile.WriteToFile(ESP_LOG_DEBUG, TAG, "Need to create parent folder first: " + parent);
                if(MakeDir(parent)) {
                    //Now, try to create again.
                    bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
				}
                else {
        			LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to create parent folder: " + parent);
                    bSuccess = false;
				}
                break;

            case EEXIST:
                //Done!
                bSuccess = true;
                break;
				
            default:
				LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Failed to create folder: " + path + " (errno: " + std::to_string(errno) + ")");
                bSuccess = false;
                break;
        }
    }
    else {
        bSuccess = true;
	}

    return bSuccess;
}


bool ctype_space(const char c, string adddelimiter)
{
	if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 11)
	{
		return true;
	}
	if (adddelimiter.find(c) != string::npos)
		return true;

	return false;
}


string trim(string istring, string adddelimiter)
{
	bool trimmed = false;

	if (ctype_space(istring[istring.length() - 1], adddelimiter))
	{
		istring.erase(istring.length() - 1);
		trimmed = true;
	}

	if (ctype_space(istring[0], adddelimiter))
	{
		istring.erase(0, 1);
		trimmed = true;
	}

	if ((trimmed == false) || (istring.size() == 0))
	{
		return istring;
	}
	else
	{
		return trim(istring, adddelimiter);
	}
}


size_t findDelimiterPos(string input, string delimiter)
{
	size_t pos = std::string::npos;
	size_t zw;
	string akt_del;

	for (int anz = 0; anz < delimiter.length(); ++anz)
	{
		akt_del = delimiter[anz];
		if ((zw = input.find(akt_del)) != std::string::npos)
		{
			if (pos != std::string::npos)
			{
				if (zw < pos)
					pos = zw;
			}
			else
				pos = zw;
		}
	}
	return pos;
}


bool RenameFile(string from, string to)
{
//	ESP_LOGI(logTag, "Deleting file: %s", fn.c_str());
	/* Delete file */
	FILE* fpSourceFile = fopen(from.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		ESP_LOGE(TAG, "DeleteFile: File %s existiert nicht!", from.c_str());
		return false;
	}
	fclose(fpSourceFile);

	rename(from.c_str(), to.c_str());
	return true;
}


bool FileExists(string filename)
{
	FILE* fpSourceFile = fopen(filename.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		return false;
	}
	fclose(fpSourceFile);
	return true;    
}


bool DeleteFile(string fn)
{
//	ESP_LOGI(logTag, "Deleting file: %s", fn.c_str());
	/* Delete file */
	FILE* fpSourceFile = fopen(fn.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		ESP_LOGD(TAG, "DeleteFile: File %s existiert nicht!", fn.c_str());
		return false;
	}
	fclose(fpSourceFile);

	unlink(fn.c_str());
	return true;    
}


bool CopyFile(string input, string output)
{
	input = FormatFileName(input);
	output = FormatFileName(output);

	if (toUpper(input).compare(WLAN_CONFIG_FILE) == 0)
	{
		ESP_LOGD(TAG, "wlan.ini kann nicht kopiert werden!");
		return false;
	}

	char cTemp;
	FILE* fpSourceFile = fopen(input.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		ESP_LOGD(TAG, "File %s existiert nicht!", input.c_str());
		return false;
	}

	FILE* fpTargetFile = fopen(output.c_str(), "wb");

	// Code Section

	// Read From The Source File - "Copy"
	while (fread(&cTemp, 1, 1, fpSourceFile) == 1)
	{
		// Write To The Target File - "Paste"
		fwrite(&cTemp, 1, 1, fpTargetFile);
	}

	// Close The Files
	fclose(fpSourceFile);
	fclose(fpTargetFile);
	ESP_LOGD(TAG, "File copied: %s to %s", input.c_str(), output.c_str());
	return true;
}


string getFileFullFileName(string filename)
{
	size_t lastpos = filename.find_last_of('/');

	if (lastpos == string::npos)
		return "";

//	ESP_LOGD(TAG, "Last position: %d", lastpos);

	string zw = filename.substr(lastpos + 1, filename.size() - lastpos);

	return zw;
}


string getDirectory(string filename)
{
	size_t lastpos = filename.find('/');

	if (lastpos == string::npos)
		lastpos = filename.find('\\');

	if (lastpos == string::npos)
		return "";

//	ESP_LOGD(TAG, "Directory: %d", lastpos);

	string zw = filename.substr(0, lastpos - 1);
	return zw;
}


string getFileType(string filename)
{
	size_t lastpos = filename.rfind(".", filename.length());
	size_t neu_pos;
	while ((neu_pos = filename.find(".", lastpos + 1)) > -1)
	{
		lastpos = neu_pos;
	}

	if (lastpos == string::npos)
		return "";

	string zw = filename.substr(lastpos + 1, filename.size() - lastpos);
	zw = toUpper(zw);

	return zw;
}


/* recursive mkdir */
int mkdir_r(const char *dir, const mode_t mode) {
    char tmp[FILE_PATH_MAX];
    char *p = NULL;
    struct stat sb;
    size_t len;
    
    /* copy path */
    len = strnlen (dir, FILE_PATH_MAX);
    if (len == 0 || len == FILE_PATH_MAX) {
        return -1;
    }
    memcpy (tmp, dir, len);
    tmp[len] = '\0';

    /* remove trailing slash */
    if(tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    /* check if path exists and is a directory */
    if (stat (tmp, &sb) == 0) {
        if (S_ISDIR (sb.st_mode)) {
            return 0;
        }
    }
    
    /* recursive mkdir */
    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            /* test path */
            if (stat(tmp, &sb) != 0) {
                /* path does not exist - create directory */
                if (mkdir(tmp, mode) < 0) {
                    return -1;
                }
            } else if (!S_ISDIR(sb.st_mode)) {
                /* not a directory */
                return -1;
            }
            *p = '/';
        }
    }
    /* test path */
    if (stat(tmp, &sb) != 0) {
        /* path does not exist - create directory */
        if (mkdir(tmp, mode) < 0) {
            return -1;
        }
    } else if (!S_ISDIR(sb.st_mode)) {
        /* not a directory */
        return -1;
    }
    return 0;
}


string toUpper(string in)
{
	for (int i = 0; i < in.length(); ++i)
		in[i] = toupper(in[i]);
	
	return in;
}


string toLower(string in)
{
	for (int i = 0; i < in.length(); ++i)
		in[i] = tolower(in[i]);
	
	return in;
}


// CPU Temp
extern "C" uint8_t temprature_sens_read();
float temperatureRead()
{
    return (temprature_sens_read() - 32) / 1.8;
}


time_t addDays(time_t startTime, int days) {
	struct tm* tm = localtime(&startTime);
	tm->tm_mday += days;
	return mktime(tm);
}


int removeFolder(const char* folderPath, const char* logTag) {
	//ESP_LOGD(logTag, "Delete content in path %s", folderPath);

	DIR *dir = opendir(folderPath);
    if (!dir) {
        ESP_LOGE(logTag, "Failed to stat dir: %s", folderPath);
        return -1;
    }

    struct dirent *entry;
    int deleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        std::string path = string(folderPath) + "/" + entry->d_name;
		if (entry->d_type == DT_REG) {
			//ESP_LOGD(logTag, "Delete file %s", path.c_str());
			if (unlink(path.c_str()) == 0) {
				deleted ++;
			} else {
				ESP_LOGE(logTag, "can't delete file: %s", path.c_str());
			}
        } else if (entry->d_type == DT_DIR) {
			deleted += removeFolder(path.c_str(), logTag);
		}
    }
    
    closedir(dir);
	if (rmdir(folderPath) != 0) {
		ESP_LOGE(logTag, "can't delete folder: %s", folderPath);
	}
	ESP_LOGD(logTag, "%d files in folder %s deleted.", deleted, folderPath);

	return deleted;
}


std::vector<string> HelperZerlegeZeile(std::string input, std::string _delimiter = "")
{
	std::vector<string> Output;
	std::string delimiter = " =,";
    if (_delimiter.length() > 0){
        delimiter = _delimiter;
    }

	return ZerlegeZeile(input, delimiter);
}


std::vector<string> ZerlegeZeile(std::string input, std::string delimiter)
{
	std::vector<string> Output;
	/* The input can have multiple formats: 
	 *  - key = value
     *  - key = value1 value2 value3 ...
     *  - key value1 value2 value3 ...
	 *  
	 * Examples:
	 *  - ImageSize = VGA
	 *  - IO0 = input disabled 10 false false 
	 *  - main.dig1 28 144 55 100 false
	 * 
	 * This causes issues eg. if a password key has a whitespace or equal sign in its value.
	 * As a workaround and to not break any legacy usage, we enforce to only use the
	 * equal sign, if the key is "password"
	*/
	if ((input.find("password") != string::npos) || (input.find("Token") != string::npos)) { // Line contains a password, use the equal sign as the only delimiter and only split on first occurrence
		size_t pos = input.find("=");
		Output.push_back(trim(input.substr(0, pos), ""));
		Output.push_back(trim(input.substr(pos +1, string::npos), ""));
	}
	else { // Legacy Mode
		input = trim(input, delimiter);							// sonst werden delimiter am Ende (z.B. == im Token) gelöscht)
		size_t pos = findDelimiterPos(input, delimiter);
		std::string token;
		while (pos != std::string::npos) {
			token = input.substr(0, pos);
			token = trim(token, delimiter);
			Output.push_back(token);
			input.erase(0, pos + 1);
			input = trim(input, delimiter);
			pos = findDelimiterPos(input, delimiter);
		}
		Output.push_back(input);
	}

	return Output;

}


std::string ReplaceString(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}


/* Source: https://git.kernel.org/pub/scm/utils/mmc/mmc-utils.git/tree/lsmmc.c */
/* SD Card Manufacturer Database */
struct SDCard_Manufacturer_database {
	string type;
	int id;
	string manufacturer;
};


/* Source: https://git.kernel.org/pub/scm/utils/mmc/mmc-utils.git/tree/lsmmc.c */
/* SD Card Manufacturer Database */
struct SDCard_Manufacturer_database sd_database[] = {
	{
		.type = "sd",
		.id = 0x01,
		.manufacturer = "Panasonic",
	},
	{
		.type = "sd",
		.id = 0x02,
		.manufacturer = "Toshiba/Kingston/Viking",
	},
	{
		.type = "sd",
		.id = 0x03,
		.manufacturer = "SanDisk",
	},
	{
		.type = "sd",
		.id = 0x08,
		.manufacturer = "Silicon Power",
	},
	{
		.type = "sd",
		.id = 0x18,
		.manufacturer = "Infineon",
	},
	{
		.type = "sd",
		.id = 0x1b,
		.manufacturer = "Transcend/Samsung",
	},
	{
		.type = "sd",
		.id = 0x1c,
		.manufacturer = "Transcend",
	},
	{
		.type = "sd",
		.id = 0x1d,
		.manufacturer = "Corsair/AData",
	},
	{
		.type = "sd",
		.id = 0x1e,
		.manufacturer = "Transcend",
	},
	{
		.type = "sd",
		.id = 0x1f,
		.manufacturer = "Kingston",
	},
	{
		.type = "sd",
		.id = 0x27,
		.manufacturer = "Delkin/Phison",
	},
	{
		.type = "sd",
		.id = 0x28,
		.manufacturer = "Lexar",
	},
	{
		.type = "sd",
		.id = 0x30,
		.manufacturer = "SanDisk",
	},
	{
		.type = "sd",
		.id = 0x31,
		.manufacturer = "Silicon Power",
	},
	{
		.type = "sd",
		.id = 0x33,
		.manufacturer = "STMicroelectronics",
	},
	{
		.type = "sd",
		.id = 0x41,
		.manufacturer = "Kingston",
	},
	{
		.type = "sd",
		.id = 0x6f,
		.manufacturer = "STMicroelectronics",
	},
	{
		.type = "sd",
		.id = 0x74,
		.manufacturer = "Transcend",
	},
	{
		.type = "sd",
		.id = 0x76,
		.manufacturer = "Patriot",
	},
	{
		.type = "sd",
		.id = 0x82,
		.manufacturer = "Gobe/Sony",
	},
	{
		.type = "sd",
		.id = 0x89,
		.manufacturer = "Unknown",
	},
};

struct SDCard_Manufacturer_database mmc_database[] = {
	{
		.type = "mmc",
		.id = 0x00,
		.manufacturer = "SanDisk",
	},
	{
		.type = "mmc",
		.id = 0x02,
		.manufacturer = "Kingston/SanDisk",
	},
	{
		.type = "mmc",
		.id = 0x03,
		.manufacturer = "Toshiba",
	},
	{
		.type = "mmc",
		.id = 0x05,
		.manufacturer = "Unknown",
	},
	{
		.type = "mmc",
		.id = 0x06,
		.manufacturer = "Unknown",
	},
	{
		.type = "mmc",
		.id = 0x11,
		.manufacturer = "Toshiba",
	},
	{
		.type = "mmc",
		.id = 0x13,
		.manufacturer = "Micron",
	},
	{
		.type = "mmc",
		.id = 0x15,
		.manufacturer = "Samsung/SanDisk/LG",
	},
	{
		.type = "mmc",
		.id = 0x37,
		.manufacturer = "KingMax",
	},
	{
		.type = "mmc",
		.id = 0x44,
		.manufacturer = "ATP",
	},
	{
		.type = "mmc",
		.id = 0x45,
		.manufacturer = "SanDisk Corporation",
	},
	{
		.type = "mmc",
		.id = 0x2c,
		.manufacturer = "Kingston",
	},
	{
		.type = "mmc",
		.id = 0x70,
		.manufacturer = "Kingston",
	},
	{
		.type = "mmc",
		.id = 0xfe,
		.manufacturer = "Micron",
	},
};

/* Parse SD Card Manufacturer Database */
string SDCardParseManufacturerIDs(int id) 
{
    if (SDCardIsMMC)
    {
        unsigned int id_cnt = sizeof(mmc_database) / sizeof(struct SDCard_Manufacturer_database);
        string ret_val = "";

        for (int i = 0; i < id_cnt; i++)
	{
	    if (mmc_database[i].id == id)
	    {
		return mmc_database[i].manufacturer;
	    }
	    else
	    {
		ret_val = "ID unknown (not in DB)";
	    }
	}

	return ret_val;
    }

    else
    {
	unsigned int id_cnt = sizeof(sd_database) / sizeof(struct SDCard_Manufacturer_database);
	string ret_val = "";

	for (int i = 0; i < id_cnt; i++)
	{
	    if (sd_database[i].id == id)
	    {
		return sd_database[i].manufacturer;
	    }
	    else
	    {
		ret_val = "ID unknown (not in DB)";
	    }
	}

	return ret_val;
    }
}


string RundeOutput(double _in, int _anzNachkomma)
{
    std::stringstream stream;
    int _zw = _in;    
//    ESP_LOGD(TAG, "AnzNachkomma: %d", _anzNachkomma);

    if (_anzNachkomma < 0) {
        _anzNachkomma = 0;
    }

    if (_anzNachkomma > 0)
    {
        stream << std::fixed << std::setprecision(_anzNachkomma) << _in;
        return stream.str();          
    }
    else
    {
        stream << _zw;
    }


    return stream.str();  
}


string getMac(void) {
    uint8_t macInt[6];
    char macFormated[6*2 + 5 + 1]; // AA:BB:CC:DD:EE:FF

    esp_read_mac(macInt, ESP_MAC_WIFI_STA);
    sprintf(macFormated, "%02X:%02X:%02X:%02X:%02X:%02X", macInt[0], macInt[1], macInt[2], macInt[3], macInt[4], macInt[5]); 

    return macFormated;
}


void setSystemStatusFlag(SystemStatusFlag_t flag) {
	systemStatus = systemStatus | flag; // set bit

	char buf[20];
	snprintf(buf, sizeof(buf), "0x%08X", getSystemStatus());
    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "New System Status: " + std::string(buf));
}


void clearSystemStatusFlag(SystemStatusFlag_t flag) {
	systemStatus = systemStatus | ~flag; // clear bit

	char buf[20];
	snprintf(buf, sizeof(buf), "0x%08X", getSystemStatus());
    LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "New System Status: " + std::string(buf));
}


int getSystemStatus(void) {
    return systemStatus;
}


bool isSetSystemStatusFlag(SystemStatusFlag_t flag) {
	//ESP_LOGE(TAG, "Flag (0x%08X) is set (0x%08X): %d", flag, systemStatus , ((systemStatus & flag) == flag));

	if ((systemStatus & flag) == flag) {
		return true;
	}
	else {
		return false;
	}
}


time_t getUpTime(void) {
    return (uint32_t)(esp_timer_get_time()/1000/1000); // in seconds
}


string getResetReason(void) {
	std::string reasonText;

	switch(esp_reset_reason()) {
		case ESP_RST_POWERON: reasonText = "Power-on event (or reset button)"; break;    //!< Reset due to power-on event
		case ESP_RST_EXT: reasonText = "External pin"; break;        //!< Reset by external pin (not applicable for ESP32)
		case ESP_RST_SW: reasonText = "Via esp_restart"; break;         //!< Software reset via esp_restart
		case ESP_RST_PANIC: reasonText = "Exception/panic"; break;      //!< Software reset due to exception/panic
		case ESP_RST_INT_WDT: reasonText = "Interrupt watchdog"; break;    //!< Reset (software or hardware) due to interrupt watchdog
		case ESP_RST_TASK_WDT: reasonText = "Task watchdog"; break;   //!< Reset due to task watchdog
		case ESP_RST_WDT: reasonText = "Other watchdogs"; break;        //!< Reset due to other watchdogs
		case ESP_RST_DEEPSLEEP: reasonText = "Exiting deep sleep mode"; break;  //!< Reset after exiting deep sleep mode
		case ESP_RST_BROWNOUT: reasonText = "Brownout"; break;   //!< Brownout reset (software or hardware)
		case ESP_RST_SDIO: reasonText = "SDIO"; break;       //!< Reset over SDIO

		case ESP_RST_UNKNOWN:   //!< Reset reason can not be determined
		default: 
			reasonText = "Unknown";
	}
    return reasonText;
}


/**
 * Returns the current uptime  formated ad xxf xxh xxm [xxs]
 */
std::string getFormatedUptime(bool compact) {
	char buf[20];
	#pragma GCC diagnostic ignored "-Wformat-truncation"

    int uptime = getUpTime(); // in seconds

    int days = int(floor(uptime / (3600*24)));
    int hours = int(floor((uptime - days * 3600*24) / (3600)));
    int minutes = int(floor((uptime - days * 3600*24 - hours * 3600) / (60)));
    int seconds = uptime - days * 3600*24 - hours * 3600 - minutes * 60;
    
	if (compact) {
		snprintf(buf, sizeof(buf), "%dd%02dh%02dm%02ds", days, hours, minutes, seconds);
	}
	else {
		snprintf(buf, sizeof(buf), "%3dd %02dh %02dm %02ds", days, hours, minutes, seconds);
	}

	return std::string(buf);
}


const char* get404(void) {
    return 
"<pre>\n\n\n\n"
"        _\n"
"    .__(.)< ( oh oh! This page does not exist! )\n"
"    \\___)\n"
"\n\n"
"                You could try your <a href=index.html target=_parent>luck</a> here!</pre>\n"
"<script>document.cookie = \"page=overview.html\"</script>"; // Make sure we load the overview page
}


std::string UrlDecode(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    for (std::size_t i = 0; i < value.size(); ++i)
    {
        auto ch = value[i];

        if (ch == '%' && (i + 2) < value.size())
        {
            auto hex = value.substr(i + 1, 2);
            auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            result.push_back(dec);
            i += 2;
        }
        else if (ch == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(ch);
        }
    }

    return result;
}


bool replaceString(std::string& s, std::string const& toReplace, std::string const& replaceWith) {
    return replaceString(s, toReplace, replaceWith, true);
}


bool replaceString(std::string& s, std::string const& toReplace, std::string const& replaceWith, bool logIt) {
    std::size_t pos = s.find(toReplace);

    if (pos == std::string::npos) { // Not found
        return false;
    }

    std::string old = s;
    s.replace(pos, toReplace.length(), replaceWith);
    if (logIt) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, "Migrated Configfile line '" + old + "' to '" + s + "'");
    }
    return true;
}


bool isInString(std::string& s, std::string const& toFind) {
    std::size_t pos = s.find(toFind);

    if (pos == std::string::npos) { // Not found
        return false;
    }
    return true;
}
