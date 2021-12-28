#include "../std_c.h"
#include "../usbfs/usbfs.h"
#include "BinFile.h"
#include "wtp.h"

char *g_DkbTimPath;
char *g_DkbFilePath;
char *g_FbfTimPath;
char *g_FbfFilePath;
static struct CBinFileWtp gBinFileWtp;

static int my_fseek(FILE *f, long off, int whence)
{
    //dprintf("off=%lx, whe=%d\n", off, whence);
    return fseek(f, off, whence);
}

static size_t my_fread(void *destv, size_t size, size_t nmemb, FILE *f) {
    int ret = fread(destv, size, nmemb, f);
    return ret;
}

static int BinFileWtp_InitParameter2(struct CBinFileWtp *me, FILE *fw_fp)
{
    int nPosBuf;
    uint32_t uValue = 0, i;

    me->m_fBinFile = fw_fp;
    my_fseek(me->m_fBinFile, -1 * sizeof(uint32_t), SEEK_END);
    my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
    //dprintf("Read from File Value is %x\n", uValue);
    uValue = le32toh(uValue);
    //dprintf("Convert Value is %x\n", uValue);
    my_fseek(me->m_fBinFile, uValue, SEEK_SET);
    //dprintf("Offset of Map is %u\n", uValue);

    my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
    uValue = le32toh(uValue);
    //dprintf("Size of Map is %u\n", uValue);
    me->nSize = uValue;
    if (me->nSize != 4) {
        dprintf("only support image number 4, but get %d\n", me->nSize);
        return 0;
    }

    for (i = 0; i < me->nSize; i++)
    {
        int j;
        char buffer[512];

        memset(buffer, 0, sizeof(buffer));

        my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
        uValue = le32toh(uValue);

        my_fread(buffer, 1, uValue, me->m_fBinFile);

        nPosBuf = 0;
        for (j = 0; j < uValue; j += 2)
        {
            me->FbfList[i][nPosBuf++] = buffer[j];
        }
        char asr_FbfList[256] = {0};
        me->FbfList[i][nPosBuf] = 0;
        while (nPosBuf > 0) {
            if (me->FbfList[i][nPosBuf] == '\\') {
                strcpy(asr_FbfList, &(me->FbfList[i][nPosBuf+1]));
                strcpy(me->FbfList[i], asr_FbfList);
            }
            nPosBuf--;
        }

        //dprintf("Find image name:%s\n", me->FbfList[i]);
        my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
        me->m_mapFile2Struct[i].Head1 = le32toh(uValue);

        my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
        me->m_mapFile2Struct[i].Head2 = le32toh(uValue);

        my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
        me->m_mapFile2Struct[i].Mid1 = le32toh(uValue);
        
        my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
        me->m_mapFile2Struct[i].Mid2 = le32toh(uValue);
        
        my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
        me->m_mapFile2Struct[i].Tail1 = le32toh(uValue);
        
        my_fread(&uValue, sizeof(uint32_t), 1, me->m_fBinFile);
        me->m_mapFile2Struct[i].Tail2 = le32toh(uValue);
        dprintf("%-24s Head={%08x, %08x}, Mid={%08x, %08x}, Tail={%08x, %08x}\n",
            me->FbfList[i],
            me->m_mapFile2Struct[i].Head1, me->m_mapFile2Struct[i].Head2,
            me->m_mapFile2Struct[i].Mid1, me->m_mapFile2Struct[i].Mid2,
            me->m_mapFile2Struct[i].Tail1, me->m_mapFile2Struct[i].Tail2);
    }
    
    return 1;
}

int BinFileWtp_FseekBin(struct CBinFileWtp *me, const char *pszBinFile, long _Offset, int _Origin)
{
    int ret;
    //uint32_t kfs, kfp;
    uint32_t i;
    FileStruct *pFile = NULL;

    //dprintf("%s file=%s, offst=%ld, orig=%d\n", __func__, pszBinFile, _Offset, _Origin);
    for (i = 0; i < me->nSize; i++)
    {
        if (!strcmp(me->FbfList[i], pszBinFile)) {
            pFile = &me->m_mapFile2Struct[i];
        }
    }

    if (!pFile) {
        dprintf("%s fail to find %s\n", __func__, pszBinFile);
        return -1;
    }

       
	//kfp = kh_get(str2n, me->m_mapFile2PointPos, pszBinFile);
	//kfs = kh_get(str2fs, me->m_mapFile2Struct, pszBinFile);
	//if (kfp == kh_end(me->m_mapFile2PointPos) || kfs == kh_end(me->m_mapFile2Struct))
	//	return -1;
	if (SEEK_SET == _Origin)
	{
		ret = my_fseek(me->m_fBinFile, pFile->Head1 + _Offset, SEEK_SET);
	}
	else if (SEEK_END == _Origin)
	{
		ret = my_fseek(me->m_fBinFile, pFile->Tail2 + _Offset, SEEK_SET);
	}
	else
	{
	    dprintf("%s unspport SEEK_CUR\n", __func__);
        ret = -1;
		//fseek(me->m_fBinFile, kh_value(me->m_mapFile2PointPos, kfp) + _Offset, SEEK_SET);
	}
	return ret;
}

size_t BinFileWtp_ReadBinFile(struct CBinFileWtp *me, void *buffer, size_t size, size_t count, const char *pszBinFile)
{
    return my_fread(buffer, size, count, me->m_fBinFile);
}

uint32_t BinFileWtp_GetFileSize(struct CBinFileWtp *me, const char *pszBinFile)
{
    uint32_t i;
    FileStruct *pFile = NULL;

    //dprintf("%s file=%s\n", __func__, pszBinFile);
    for (i = 0; i < me->nSize; i++)
    {
        if (!strcmp(me->FbfList[i], pszBinFile)) {
            pFile = &me->m_mapFile2Struct[i];
        }
    }

    if (!pFile) {
        dprintf("%s fail to find %s\n", __func__, pszBinFile);
        return -1;
    }

    i = pFile->Head2 - pFile->Head1 + pFile->Mid2 - pFile->Mid1 +pFile->Tail2 - pFile->Tail1;
     
	return i;
}

static void EndianConvertMasterBlockHeader(MasterBlockHeader *mbHeader)
{
	int i;
	for (i = 0; i < NUM_OF_SUPPORTED_FLASH_DEVS; i++){
		mbHeader->Flash_Device_Spare_Area_Size[i] = le16toh(mbHeader->Flash_Device_Spare_Area_Size[i]);
	}
	mbHeader->Format_Version = le16toh(mbHeader->Format_Version);
	mbHeader->Size_of_Block = le16toh(mbHeader->Size_of_Block);
	mbHeader->Bytes_To_Program = le32toh(mbHeader->Bytes_To_Program);
	mbHeader->Bytes_To_Verify = le32toh(mbHeader->Bytes_To_Verify);
	mbHeader->Number_of_Bytes_To_Erase = le32toh(mbHeader->Number_of_Bytes_To_Erase);
	mbHeader->Main_Commands = le32toh(mbHeader->Main_Commands);
	mbHeader->nOfDevices = le32toh(mbHeader->nOfDevices);
	mbHeader->DLerVersion = le32toh(mbHeader->DLerVersion);
	for (i = 0; i < MAX_NUMBER_OF_FLASH_DEVICES_IN_MASTER_HEADER; i++){
		mbHeader->deviceHeaderOffset[i] = le32toh(mbHeader->deviceHeaderOffset[i]);
	}
}

static void EndianConvertDeviceHeader_V11(DeviceHeader_V11 *pDeviceHeader)
{
	int i;
	pDeviceHeader->DeviceFlags = le32toh(pDeviceHeader->DeviceFlags);
	for (i = 0; i < 16; i++){
		pDeviceHeader->DeviceParameters[i] = le32toh(pDeviceHeader->DeviceParameters[i]);
	}
	pDeviceHeader->FlashOpt.EraseAll = le32toh(pDeviceHeader->FlashOpt.EraseAll);
	pDeviceHeader->FlashOpt.ResetBBT = le32toh(pDeviceHeader->FlashOpt.ResetBBT);
	pDeviceHeader->FlashOpt.NandID = le32toh(pDeviceHeader->FlashOpt.NandID);
	for (i = 0; i < MAX_RESEVERD_LEN-1; i++){
		pDeviceHeader->FlashOpt.Reserved[i] = le32toh(pDeviceHeader->FlashOpt.Reserved[i]);
	}
	pDeviceHeader->FlashOpt.Skip_Blocks_Struct.Total_Number_Of_SkipBlocks = le32toh(pDeviceHeader->FlashOpt.Skip_Blocks_Struct.Total_Number_Of_SkipBlocks);
	for (i = 0; i < MAX_NUM_SKIP_BLOCKS; i++){
		pDeviceHeader->FlashOpt.Skip_Blocks_Struct.Skip_Blocks[i] = le32toh(pDeviceHeader->FlashOpt.Skip_Blocks_Struct.Skip_Blocks[i]);
	}
	pDeviceHeader->ProductionMode = le32toh(pDeviceHeader->ProductionMode);
	for (i = 0; i < MAX_RESEVERD_LEN-2; i++){
		pDeviceHeader->Reserved[i] = le32toh(pDeviceHeader->Reserved[i]);
	}
	pDeviceHeader->nOfImages = le32toh(pDeviceHeader->nOfImages);
	for (i = 0; i < MAX_NUMBER_OF_IMAGE_STRUCTS_IN_DEVICE_HEADER; i++) {
        int j;

        pDeviceHeader->imageStruct[i].Image_ID = le32toh(pDeviceHeader->imageStruct[i].Image_ID);
		pDeviceHeader->imageStruct[i].Image_In_TIM = le32toh(pDeviceHeader->imageStruct[i].Image_In_TIM);
		pDeviceHeader->imageStruct[i].Flash_partition = le32toh(pDeviceHeader->imageStruct[i].Flash_partition);
		pDeviceHeader->imageStruct[i].Flash_erase_size = le32toh(pDeviceHeader->imageStruct[i].Flash_erase_size);
		pDeviceHeader->imageStruct[i].commands = le32toh(pDeviceHeader->imageStruct[i].commands);
		pDeviceHeader->imageStruct[i].First_Sector = le32toh(pDeviceHeader->imageStruct[i].First_Sector);
		pDeviceHeader->imageStruct[i].length = le32toh(pDeviceHeader->imageStruct[i].length);
		pDeviceHeader->imageStruct[i].Flash_Start_Address = le32toh(pDeviceHeader->imageStruct[i].Flash_Start_Address);
		for (j = 0; j < MAX_RESEVERD_LEN; j++){
			pDeviceHeader->imageStruct[i].Reserved[j] = le32toh(pDeviceHeader->imageStruct[i].Reserved[j]);
		}
		pDeviceHeader->imageStruct[i].ChecksumFormatVersion2 = le32toh(pDeviceHeader->imageStruct[i].ChecksumFormatVersion2);
	}
}

static int InitInstanceParams(struct CBinFileWtp *me, PInstanceParams lpInstanceParam) {
    int i;
    char *strFile;

    for (i = 0; i < me->nSize; i++)
    {
        strFile = me->FbfList[i];
        //dprintf("Init %s\n", me->FbfList[i]);

    	if (strstr(strFile, "DKB_timheader.bin") != NULL || strstr(strFile, "DKB_ntimheader.bin") != NULL)
    	    g_DkbTimPath = strFile;
        else if (strstr(strFile,"Dkb.bin"))
    	    g_DkbFilePath = strFile;
    	else if (strstr(strFile, "FBF_timheader.bin") != NULL || strstr(strFile, "FBF_Ntimheader.bin") != NULL)
    	    g_FbfTimPath = strFile;
        else if (strstr(strFile,"FBF_h.bin"))
    	    g_FbfFilePath = strFile;
        else
        {
            dprintf("Un-support file %s\n", me->FbfList[i]);
            return 0;
        }
    }

    lpInstanceParam->pszDKBTim = g_DkbTimPath;
    lpInstanceParam->pszDKBbin = g_DkbFilePath;

    //dprintf("DKB_timheader: %24s  DKB: %s\n", g_DkbTimPath, g_DkbFilePath);
    //dprintf("FBF head:      %24s  FBF: %s\n", g_FbfTimPath, g_FbfFilePath);
    assert(g_FbfFilePath != NULL);

    BinFileWtp_FseekBin(me, g_FbfFilePath, 4L, SEEK_SET);
    BinFileWtp_ReadBinFile(me,&me->masterBlkHeader,sizeof(MasterBlockHeader),1, g_FbfFilePath);
    EndianConvertMasterBlockHeader(&me->masterBlkHeader);
    //dprintf("MasterBlkHeader.SizeOfBlock=%x\n",me->masterBlkHeader.Size_of_Block);

    BinFileWtp_FseekBin(me, g_FbfFilePath, me->masterBlkHeader.deviceHeaderOffset[0] + 4,SEEK_SET);
    BinFileWtp_ReadBinFile(me,&me->deviceHeaderBuf,sizeof(DeviceHeader_V11), 1, g_FbfFilePath);
    EndianConvertDeviceHeader_V11(&me->deviceHeaderBuf);
    //dprintf("deviceHeaderBuf.DeviceFlags=%x\n",me->deviceHeaderBuf.DeviceFlags);

    lpInstanceParam->PlaformType = me->deviceHeaderBuf.ChipID;
    if (lpInstanceParam->PlaformType != PLAT_NEZHA3) {
       dprintf("only support PLAT_NEZHA3\n");
       return 0;
    }
    lpInstanceParam->Commands |= (me->deviceHeaderBuf.FlashOpt.EraseAll?1:0);
    lpInstanceParam->Commands |= (me->deviceHeaderBuf.FlashOpt.ResetBBT? (1<<2):0);
    if (me->deviceHeaderBuf.FlashOpt.NandID)
    {
        lpInstanceParam->FlashType = 0;
    }
    /* Note: the BBCS and CRCS on/off are controlled by the blf via which fbf file was created.
        Here for fbfdownloader pc side, just assume both are ON in order to avoid extract the information from TIMH.
        The CallbackProc only try to read the infos when BBCS or CRCS success although the on/off at PC are always set to ON
        lpInstanceParam->GetBadBlk = 1;
        lpInstanceParam->ImageCrcEn = 1;
    */
    lpInstanceParam->GetBadBlk = me->deviceHeaderBuf.BBCS_EN;
    lpInstanceParam->ImageCrcEn = me->deviceHeaderBuf.CRCS_EN;
	        
    return 0;
}

static BOOL WtptpDownLoad_ParseTim (struct CBinFileWtp *me, const char *strTimFile, const char *strImageFile)
{
    UINT32 dwTimId;
    UINT32 dwImageId;	
    CTIM TimHeader;
    uint32_t Version;
    int i, nImages;

    BinFileWtp_FseekBin(me, strTimFile, sizeof(UINT32), SEEK_SET);
    BinFileWtp_ReadBinFile(me,&dwTimId,sizeof(UINT32) , 1, strTimFile);
    dwTimId = le32toh(dwTimId);
    dprintf("dwTimId: %x\n", dwTimId);
    if (TIMH != dwTimId) {
        return 0;
    }

    BinFileWtp_FseekBin(me, strTimFile, 0L, SEEK_SET);
    BinFileWtp_ReadBinFile(me, &TimHeader, sizeof(CTIM), 1, strTimFile);
    nImages = le32toh(TimHeader.NumImages);
    dprintf("Number of Images listed in TIM: %d\n",nImages);
    if (nImages != 2) {
        dprintf("Number of Images is unknown:%d\n", nImages);
        return 0;
    }

    dprintf("\t{Version = %x, Identifier = %x, Trusted = %x, IssueDate = %x, OEMUniqueID = %x}\n",
        TimHeader.VersionBind.Version, TimHeader.VersionBind.Identifier, TimHeader.VersionBind.Trusted,
        TimHeader.VersionBind.IssueDate, TimHeader.VersionBind.OEMUniqueID);

    Version = le32toh(TimHeader.VersionBind.Version);
    if (Version >= TIM_3_1_01 && le32toh(TimHeader.VersionBind.Version) < TIM_3_2_00)
    {
        IMAGE_INFO_3_1_0 ImageInfo;
        for (i = 0; i < nImages; i++)
        {
            BinFileWtp_ReadBinFile(me,&ImageInfo,sizeof(IMAGE_INFO_3_1_0),1,strTimFile);
        }
    }
    else if (Version >= TIM_3_2_00 && Version < TIM_3_4_00)
    {
        IMAGE_INFO_3_2_0 ImageInfo;
        for (i = 0; i < nImages; i++)
        {
            BinFileWtp_ReadBinFile(me,&ImageInfo,sizeof(IMAGE_INFO_3_2_0),1,strTimFile);
        }
    }
    else if(TIM_3_4_00 == Version)
    {
        IMAGE_INFO_3_4_0 ImageInfo;
        for (i = 0; i < nImages; i++)
        {
            BinFileWtp_ReadBinFile(me,&ImageInfo,sizeof(IMAGE_INFO_3_4_0),1,strTimFile);

            dprintf("\t{ImageID = %x, NextImageID = %x, FlashEntryAddr = %x, LoadAddr = %x, ImageSize = %x}\n",
                ImageInfo.ImageID, ImageInfo.NextImageID, ImageInfo.FlashEntryAddr, ImageInfo.LoadAddr, ImageInfo.ImageSize);
        }
    }
    else if(TIM_3_5_00 == le32toh(TimHeader.VersionBind.Version))
    {
        IMAGE_INFO_3_5_0 ImageInfo;
        for (i = 0; i < nImages; i++)
        {
            BinFileWtp_ReadBinFile(me,&ImageInfo,sizeof(IMAGE_INFO_3_5_0),1,strTimFile);
        }
    }
    else
    {	
        dprintf("Tim Version is unknown:0X%X\n", TimHeader.VersionBind.Version);
        return FALSE;
    }

    BinFileWtp_FseekBin(me, strImageFile,0L,SEEK_SET);
    BinFileWtp_ReadBinFile(me,&dwImageId, sizeof(UINT32), 1, strImageFile);
    dwImageId = le32toh(dwImageId);
    dprintf("dwImageId: %x\n", dwImageId);

    return 1;
}

static BOOL WtptpDownLoad_InitializeBL(struct CBinFileWtp *me,PInstanceParams pInstParam) {
    //me->m_nFlashDataPageSize = pInstParam->FlashPageSize;
    me->m_ePlatformType = (ePlatForm)pInstParam->PlaformType;
    //me->m_FlashType = (eFlashType)pInstParam->FlashType;

    dprintf("Parse DKB Tim image.\n");
    WtptpDownLoad_ParseTim(me, g_DkbTimPath, g_DkbFilePath);
    dprintf("Parse FBF Tim image.\n");
    WtptpDownLoad_ParseTim(me, g_FbfTimPath, g_FbfFilePath);

    return 0;
}

int fbfdownloader_main(int usbfd, FILE *fw_fp, const char *modem_name)
{
    struct CBinFileWtp *me = &gBinFileWtp;
    int ret;

   me->usbfd = usbfd;
   BinFileWtp_InitParameter2(me, fw_fp);
   InitInstanceParams(me, &me->instanceParam);
   WtptpDownLoad_InitializeBL(me, &me->instanceParam);
   ret =  WtptpDownLoad_GetDeviceBootType(me);

   return ret; 
}
