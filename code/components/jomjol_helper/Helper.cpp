//#pragma warning(disable : 4996)

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Helper.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <dirent.h>
#ifdef __cplusplus
}
#endif

#include <string.h>
#include <esp_log.h>


#include "ClassLogFile.h"

#include "esp_vfs_fat.h"

static const char* TAG = "helper";

//#define ISWINDOWS_TRUE
#define PATH_MAX_STRING_SIZE 256

using namespace std;

sdmmc_cid_t SDCardCid;
sdmmc_csd_t SDCardCsd;

/////////////////////////////////////////////////////////////////////////////////////////////
string getESPHeapInfo(){
	string espInfoResultStr = "";
	char aMsgBuf[80];
    
	multi_heap_info_t aMultiHead_info ;
	heap_caps_get_info (&aMultiHead_info,MALLOC_CAP_8BIT);
	size_t aFreeHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT);
	size_t aMinFreeHeadSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
	size_t aMinFreeHeapSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
	size_t aHeapLargestFreeBlockSize = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
	sprintf(aMsgBuf," Free Heap Size: %ld", (long) aFreeHeapSize);
	size_t aFreeSPIHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_SPIRAM);
 	size_t aFreeInternalHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_INTERNAL);
	 size_t aMinFreeInternalHeapSize =  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_INTERNAL);

	sprintf(aMsgBuf," Heap: %ld", (long) aFreeHeapSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," Min Free: %ld", (long) aMinFreeHeapSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," larg. Block:  %ld", (long) aHeapLargestFreeBlockSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," SPI Heap: %ld", (long) aFreeSPIHeapSize);
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," Min Free Heap Size: %ld", (long) aMinFreeHeadSize);
	sprintf(aMsgBuf," NOT_SPI Heap: %ld", (long) (aFreeHeapSize - aFreeSPIHeapSize));
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," largest Block Size:  %ld", (long) aHeapLargestFreeBlockSize);
	sprintf(aMsgBuf," Internal Heap: %ld", (long) (aFreeInternalHeapSize));
	espInfoResultStr += string(aMsgBuf);
	sprintf(aMsgBuf," Internal Min Heap free: %ld", (long) (aMinFreeInternalHeapSize));
	espInfoResultStr += string(aMsgBuf);
	return 	espInfoResultStr;
}


size_t getESPHeapSize(){
   size_t aFreeHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT);
   return aFreeHeapSize;
}

size_t getInternalESPHeapSize() {
	size_t aFreeInternalHeapSize  = heap_caps_get_free_size(MALLOC_CAP_8BIT| MALLOC_CAP_INTERNAL);
	return aFreeInternalHeapSize;
}

string getSDCardPartitionSize(){
	FATFS *fs;
    uint32_t fre_clust, tot_sect;

    /* Get volume information and free clusters of drive 0 */
    f_getfree("0:", (DWORD *)&fre_clust, &fs);
    tot_sect = ((fs->n_fatent - 2) * fs->csize) /1024 /(1024/SDCardCsd.sector_size);	//corrected by SD Card sector size (usually 512 bytes) and convert to MB

    printf("%d MB total drive space (Sector size [bytes]: %d)\n", (int)tot_sect, (int)fs->csize*512);

	return std::to_string(tot_sect);
}

string getSDCardFreeParitionSpace(){
	FATFS *fs;
    uint32_t fre_clust, fre_sect;
  
    /* Get volume information and free clusters of drive 0 */
    f_getfree("0:", (DWORD *)&fre_clust, &fs);
    fre_sect = (fre_clust * fs->csize) / 1024 /(1024/SDCardCsd.sector_size);	//corrected by SD Card sector size (usually 512 bytes) and convert to MB

    printf("%d MB free drive space (Sector size [bytes]: %d)\n", (int)fre_sect, (int)fs->ssize);

	return std::to_string(fre_sect);
}

string getSDCardParitionAllocationSize(){
	FATFS *fs;
    uint32_t fre_clust, allocation_size;
  
    /* Get volume information and free clusters of drive 0 */
    f_getfree("0:", (DWORD *)&fre_clust, &fs);
    allocation_size = fs->ssize;

    printf("SD Card Parition Allocation Size (bytes): %d)\n", allocation_size);

	return std::to_string(allocation_size);
}


void SaveSDCardInfo(sdmmc_card_t* card) {
	SDCardCid = card->cid;
    SDCardCsd = card->csd;
}

string getSDCardManufacturer(){
	string SDCardManufacturer = SDCardParseManufacturerIDs(SDCardCid.mfg_id);
	printf("SD Card Manufactuer: %s\n", SDCardManufacturer.c_str());
	
	return (SDCardManufacturer + " (ID: " + std::to_string(SDCardCid.mfg_id) + ")");
}

string getSDCardName(){
	char *SDCardName = SDCardCid.name;
	printf("SD Card Name: %s\n", SDCardName); 

	return std::string(SDCardName);
}

string getSDCardCapacity(){
	int SDCardCapacity = SDCardCsd.capacity / (1024/SDCardCsd.sector_size) / 1024;  // total sectors * sector size  --> Byte to MB (1024*1024)
	printf("SD Card Capacity: %s\n", std::to_string(SDCardCapacity).c_str()); 

	return std::to_string(SDCardCapacity);
}

string getSDCardSectorSize(){
	int SDCardSectorSize = SDCardCsd.sector_size;
	printf("SD Card Sector Size: %s\n", std::to_string(SDCardSectorSize).c_str()); 

	return std::to_string(SDCardSectorSize);
}

/*
    int SDCardManu = card->cid.mfg_id;
    printf("SD Manu: %s\n", std::to_string(SDCardManu).c_str()); 

    printf("SD Manu (ASCII): %s\n", SDCardParseManufacturerIDs(card->cid.mfg_id).c_str());
    
    int SDCardOEM = card->cid.oem_id;
    printf("SD OEM: %s\n", std::to_string(SDCardOEM).c_str()); 

    int SDCardRev = card->cid.revision;
    printf("SD Rev: %s\n", std::to_string(SDCardRev).c_str()); 

    char *SDCardName = card->cid.name;
    printf("SD Name: %s\n", SDCardName); 

    int SDCardCSize = card->csd.sector_size;
    printf("SD C-Size: %s\n", std::to_string(SDCardCSize).c_str()); 

    int SDCardCapa = card->csd.capacity / (1024/card->csd.sector_size) / 1024;  // total sectors * sector size  --> Byte to MB (1024*1024)
    printf("SD Capa: %s\n", std::to_string(SDCardCapa).c_str()); 
*/

///////////////////////////////////////////////////////////////////////////////////////////////

void memCopyGen(uint8_t* _source, uint8_t* _target, int _size)
{
    for (int i = 0; i < _size; ++i)
        *(_target + i) = *(_source + i);
}



FILE* OpenFileAndWait(const char* nm, const char* _mode, int _waitsec)
{
	FILE *pfile;

	ESP_LOGD(TAG, "open file %s in mode %s", nm, _mode);

	if ((pfile = fopen(nm, _mode)) != NULL) {
		ESP_LOGD(TAG, "File %s successfully opened", nm);
	}
	else {
		ESP_LOGD(TAG, "Error: file %s does not exist!", nm);
		return NULL;
	}

/*
	if (pfile == NULL)
	{
		TickType_t xDelay;
		xDelay = _waitsec * 1000 / portTICK_PERIOD_MS;
		std::string zw = "File is locked: " + std::string(nm) + " - wait for " + std::to_string(_waitsec) + " seconds";
	    LogFile.WriteToFile(zw);
		vTaskDelay( xDelay );
		pfile = fopen(nm, _mode);
	}
*/

	return pfile;
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

void MakeDir(std::string _what)
{
//	chdir(_where.c_str());

	if (mkdir(_what.c_str(), S_IRWXU|S_IRWXG|S_IROTH))
		ESP_LOGD(TAG, "Problem with MakeDir: %s", _what.c_str());
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


void RenameFile(string from, string to)
{
//	ESP_LOGI(logTag, "Deleting file : %s", fn.c_str());
	/* Delete file */
	FILE* fpSourceFile = OpenFileAndWait(from.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		ESP_LOGD(TAG, "DeleteFile: File %s existiert nicht!", from.c_str());
		return;
	}
	fclose(fpSourceFile);

	rename(from.c_str(), to.c_str());
}


void DeleteFile(string fn)
{
//	ESP_LOGI(logTag, "Deleting file : %s", fn.c_str());
	/* Delete file */
	FILE* fpSourceFile = OpenFileAndWait(fn.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		ESP_LOGD(TAG, "DeleteFile: File %s existiert nicht!", fn.c_str());
		return;
	}
	fclose(fpSourceFile);

	unlink(fn.c_str());    
}


void CopyFile(string input, string output)
{
	input = FormatFileName(input);
	output = FormatFileName(output);

	if (toUpper(input).compare("/SDCARD/WLAN.INI") == 0)
	{
		ESP_LOGD(TAG, "wlan.ini kann nicht kopiert werden!");
		return;
	}

	char cTemp;
	FILE* fpSourceFile = OpenFileAndWait(input.c_str(), "rb");
	if (!fpSourceFile)	// Sourcefile existiert nicht sonst gibt es einen Fehler beim Kopierversuch!
	{
		ESP_LOGD(TAG, "File %s existiert nicht!", input.c_str());
		return;
	}

	FILE* fpTargetFile = OpenFileAndWait(output.c_str(), "wb");

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
    char tmp[PATH_MAX_STRING_SIZE];
    char *p = NULL;
    struct stat sb;
    size_t len;
    
    /* copy path */
    len = strnlen (dir, PATH_MAX_STRING_SIZE);
    if (len == 0 || len == PATH_MAX_STRING_SIZE) {
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
	ESP_LOGI(logTag, "Delete folder %s", folderPath);

	DIR *dir = opendir(folderPath);
    if (!dir) {
        ESP_LOGI(logTag, "Failed to stat dir : %s", folderPath);
        return -1;
    }

    struct dirent *entry;
    int deleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        std::string path = string(folderPath) + "/" + entry->d_name;
		if (entry->d_type == DT_REG) {
			if (unlink(path.c_str()) == 0) {
				deleted ++;
			} else {
				ESP_LOGE(logTag, "can't delete file : %s", path.c_str());
			}
        } else if (entry->d_type == DT_DIR) {
			deleted += removeFolder(path.c_str(), logTag);
		}
    }
    
    closedir(dir);
	if (rmdir(folderPath) != 0) {
		ESP_LOGE(logTag, "can't delete file : %s", folderPath);
	}
	ESP_LOGI(logTag, "%d older log files in folder %s deleted.", deleted, folderPath);

	return deleted;
}


std::vector<string> HelperZerlegeZeile(std::string input, std::string _delimiter = "")
{
	std::vector<string> Output;
	std::string delimiter = " =,";
    if (_delimiter.length() > 0){
        delimiter = _delimiter;
    }

	input = trim(input, delimiter);
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

	return Output;
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
struct SDCard_Manufacturer_database database[] = {
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
	}
};

/* Parse SD Card Manufacturer Database */
string SDCardParseManufacturerIDs(int id) 
{
	unsigned int id_cnt = sizeof(database) / sizeof(struct SDCard_Manufacturer_database);
	string ret_val = "";

	for (int i = 0; i < id_cnt; i++) {
		if (database[i].id == id) {
			return database[i].manufacturer;
		}
		else {
			ret_val = "ID unknown (not in DB)";
		}
	}
	return ret_val;
}

