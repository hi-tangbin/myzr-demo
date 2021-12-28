#include "../std_c.h"
#include "../usbfs/usbfs.h"
#include "BinFile.h"
#include "wtp.h"

#define _T
static const NOTIFICATIONTABLE WTPTPNotificationMsg[] =
{
    {PlatformBusy,_T("Platform is busy")},
    {PlatformReady,_T("Platform is Ready")},
    {PlatformResetBBT,_T("Reset BBT")},
    {PlatformEraseAllFlash,_T("Erase All flash begin")},
    {PlatformDisconnect,_T("platform is disconnect")},
    {PlatformEraseAllFlashDone,_T("Erase All flash done")},
    {PlatformEraseAllFlashWithoutBurn,_T("Erase All flash without burning begin")},
    {PlatformEraseAllFlashWithoutBurnDone,_T("Erase All flash without burning Done")},
    {PlatformFuseOperationStart,_T("Platform Fuse Operation Start")},
    {PlatformFuseOperationDone,_T("Platform Fuse Operation Done")},
    {PlatformUEInfoStart,_T("Platform UE Info Operation Start")},
    {PlatformUEInfoDone,_T("Platform UE Info Operation Done")}
};
static WTPSTATUS gWtpStatus;

static unsigned int update_size = 0;

static int WriteUSB(struct CBinFileWtp *me,WTPCOMMAND* pWtpCmd,size_t dwSize)
{
    int ret = sendSync(me->usbfd, pWtpCmd, dwSize, 0);
    return (ret == dwSize);
}

static int WriteUSBWithNoRsp (struct CBinFileWtp *me,BYTE *pData,size_t dwSize)
{
    int ret = sendSync(me->usbfd, pData, dwSize, 0);
    return (ret == dwSize);
}

static int ReadUSB (struct CBinFileWtp *me,WTPSTATUS *pWtpStatus)
{
    int bRes = TRUE;
    int dwBytes = -1;

    memset(pWtpStatus,0,16);
    dwBytes = recvSync(me->usbfd,pWtpStatus,DATALENGTH, 5000);
    if(dwBytes <= 0)
        bRes = FALSE;

    return bRes;
}

static int SendWtpMessage(struct CBinFileWtp *me, WTPCOMMAND *pWtpCmd, WTPSTATUS *pWtpStatus)
{
    size_t dwSize;

    if (pWtpCmd->CMD == PREAMBLE)
        dwSize  = 4;
    else
        dwSize = 8 + le32toh(pWtpCmd->LEN);

    if (!WriteUSB (me,pWtpCmd,dwSize)) {
        dprintf("CMD: 0x%x - WriteUSB failed\n", pWtpCmd->CMD);
        return FALSE;
    }

    memset(pWtpStatus, 0, sizeof(WTPSTATUS) );
    if (!ReadUSB(me,pWtpStatus))
    {
        dprintf("CMD: 0x%x - ReadUSB failed\n", pWtpCmd->CMD);
        return FALSE;
    }

    if (pWtpCmd->CMD != pWtpStatus->CMD)
    {
        dprintf("CMD: 0x%x - Get CMD %d failed\n", pWtpCmd->CMD, pWtpStatus->CMD);
        return FALSE;
    }

    if ((pWtpCmd->CMD == PREAMBLE) || (pWtpCmd->CMD == MESSAGE && pWtpStatus->Status == NACK))
        ;
    else if (pWtpStatus->Status != ACK)
    {
        dprintf("CMD: 0x%x - Get Status %d failed\n", pWtpCmd->CMD, pWtpStatus->Status);
    }

    if ((pWtpStatus->Flags & 0x1 ) == MESSAGEPENDING ) {
        dprintf("message pending\n");
    }
    
    return TRUE;
}

static int SendPreamble (struct CBinFileWtp *me, WTPSTATUS* pWtpStatus)
{
    int IsOk;
    WTPCOMMAND WtpCmd = {PREAMBLE,0xD3,0x02,0x2B,htole32(0x00D3022B)};

    dprintf("WtpCmd: %s\n", __func__);
    IsOk = SendWtpMessage(me,&WtpCmd,pWtpStatus);
    if(IsOk)
    {
        IsOk = (pWtpStatus->Status == ACK && pWtpStatus->SEQ != 0xD3 && pWtpStatus->CID != 0x02);
    }

    return IsOk;
}

static int GetWtpMessage (struct CBinFileWtp *me,WTPSTATUS *pWtpStatus)
{
    int IsOk ;
    WTPCOMMAND WtpCmd = {MESSAGE,0,0x00,0x00,htole32(0x00000000)};

    IsOk = SendWtpMessage(me, &WtpCmd, pWtpStatus);
    if (!IsOk)
        return IsOk;

    if (pWtpStatus->DLEN == 6)
    {
        int i;
        uint32_t ReturnCode = le32toh(*(uint32_t *)&pWtpStatus->Data[2]);

        if (pWtpStatus->Data[0] == WTPTPNOTIFICATION)
        {
            int bFound = FALSE;

            for (i = 0; i < (sizeof(WTPTPNotificationMsg)/sizeof(WTPTPNotificationMsg[0])); i++)
            {
                if (WTPTPNotificationMsg[i].NotificationCode == ReturnCode)
                {
                    dprintf("Notification: %s 0x%X\n",WTPTPNotificationMsg[i].Description,ReturnCode);
                    bFound = TRUE;
                    break;;
                }
            }

            if(!bFound)
            {
                dprintf("Notification Code: 0x%X\n",ReturnCode);
            }
        }
        else if (pWtpStatus->Data[0]== WTPTPERRORREPORT)
        {
            dprintf("User customized ErrorCode: 0x%X\n", ReturnCode);
        }
        else if (WTPTPNOTIFICATION_UPLOAD == pWtpStatus->Data[0])
        {
            dprintf("Flash_NandID: 0x%X\n",ReturnCode);
        }
        else if (WTPTPNOTIFICATION_FLASHSIZE == pWtpStatus->Data[0])
        {
            dprintf("Flash_Size: 0x%X bytes\n",ReturnCode);
        }
        else if (WTPTPNOTIFICATION_BURNTSIZE == pWtpStatus->Data[0])
        {
            static uint32_t Burnt_Size = 0;
            // make a round to burnt size to multiple of 4096(FBF_Sector size)
            if(ReturnCode%4096)
            {
                uint32_t pad_size = 4096 - (ReturnCode%4096);
                ReturnCode += pad_size;
            }
            update_size = ReturnCode;
            Burnt_Size += ReturnCode;
            dprintf("Burnt_Size: %d bytes\n", Burnt_Size);
        }
        else {
            dprintf("WTPTP Data: 0x%X\n",pWtpStatus->Data[0]);
        }
    }
    else if (pWtpStatus->DLEN != 0)
    {
        dprintf("WTPTP DLEN: %d\n",pWtpStatus->DLEN);
    }

    return IsOk;
}

static int GetVersion (struct CBinFileWtp *me,WTPSTATUS *pWtpStatus)
{
    int IsOk;
    WTPCOMMAND WtpCmd = {GETVERSION,0,0x00,0x00,htole32(0x00000000)};

    dprintf("WtpCmd: %s\n", __func__);
    IsOk = SendWtpMessage(me, &WtpCmd, pWtpStatus);
    if (IsOk) {
        IsOk = (pWtpStatus->Status == ACK);
    }

    return IsOk;
}

static int SelectImage (struct CBinFileWtp *me, WTPSTATUS* pWtpStatus, UINT32 *pImageType)
{
    int IsOk = TRUE;
    WTPCOMMAND WtpCmd = {SELECTIMAGE,0x00,0x00,0x00,htole32(0x00000000)};

    dprintf("WtpCmd: %s\n", __func__);
    IsOk = SendWtpMessage(me, &WtpCmd, pWtpStatus);
    if (IsOk) {
        IsOk = (pWtpStatus->Status == ACK && le32toh(pWtpStatus->DLEN) == 4);
        if (IsOk) {
            *pImageType = le32toh(*(uint32_t *)&pWtpStatus->Data[0]);
        }
    }

    return IsOk;
}

static int VerifyImage (struct CBinFileWtp *me, WTPSTATUS* pWtpStatus, BYTE AckOrNack)
{
    int IsOk;
    WTPCOMMAND WtpCmd = {VERIFYIMAGE,0,0x00,0x00,htole32(0x00000001),{AckOrNack}};

    dprintf("WtpCmd: %s ack=%d\n", __func__, AckOrNack);
    IsOk = SendWtpMessage(me, &WtpCmd, pWtpStatus);
    if (IsOk) {
        IsOk = (pWtpStatus->Status == ACK);
    }

    return IsOk;
}

static int DataHeader (struct CBinFileWtp *me, WTPSTATUS* pWtpStatus, unsigned int uiRemainingData, uint32_t *uiBufferSize)
{
    int IsOk;
    WTPCOMMAND WtpCmd = {DATAHEADER,++me->m_iSequence,0x00,0x00,htole32(4)};

    dprintf("WtpCmd: %s uiRemainingData=%u\n", __func__, uiRemainingData);
    set_transfer_allbytes(uiRemainingData);

    WtpCmd.Flags = 0x4;
    *(uint32_t *)&WtpCmd.Data[0] = htole32(uiRemainingData);

    IsOk = SendWtpMessage(me, &WtpCmd, pWtpStatus);
    if (IsOk) {
        IsOk = (pWtpStatus->Status == ACK && le32toh(pWtpStatus->DLEN) == 4);
        if (IsOk) {
            *uiBufferSize = le32toh(*(uint32_t *)&pWtpStatus->Data[0]);
        }
    }

    return IsOk;
}

static int Data (struct CBinFileWtp *me, WTPSTATUS* pWtpStatus, uint8_t *pData,UINT32 Length, int isLastData)
{
    int IsOk;

    if (isLastData)
        dprintf("WtpCmd: %s Length=%u, isLastData=%d\n", __func__, Length, isLastData);

    IsOk = WriteUSBWithNoRsp (me,pData,(DWORD)Length);
    if (IsOk) {
        if (isLastData) {
            IsOk = ReadUSB (me,pWtpStatus);
            if (IsOk) {
                IsOk = (pWtpStatus->CMD == DATA && pWtpStatus->Status == ACK);
            }
        }
    }

    return IsOk;
}

static int Done (struct CBinFileWtp *me,WTPSTATUS* pWtpStatus)
{
    int IsOk;
    WTPCOMMAND WtpCmd = {DONE,0,0x00,0x00,htole32(0x00000000)};

    dprintf("WtpCmd: %s\n", __func__);
    IsOk = SendWtpMessage (me, &WtpCmd, pWtpStatus);
    if (IsOk) {
        IsOk = (pWtpStatus->Status == ACK);
    }

    return IsOk;
}

static int Disconnect (struct CBinFileWtp *me,WTPSTATUS* pWtpStatus)
{
    int IsOk;
    WTPCOMMAND WtpCmd = {DISCONNECT,0,0x00,0x00,htole32(0x00000000)};

    dprintf("WtpCmd: %s\n", __func__);
    IsOk = SendWtpMessage (me, &WtpCmd, pWtpStatus);
    if (IsOk) {
        IsOk = (pWtpStatus->Status == ACK);
    }

    return IsOk;
}

static int WtptpDownLoad_DownloadImage(struct CBinFileWtp *me, WTPSTATUS *pWtpStatus)
{
    UINT32 ImageType = 0;
    uint32_t uiBufferSize, lFileSize, uiRemainingBytes;
    const char *szFileName = NULL;
    BYTE* pSendDataBuffer;

    SelectImage(me, pWtpStatus,  &ImageType);
    dprintf("\tImageType: %x\n", ImageType);

    switch (ImageType) {
        case TIMH:
            szFileName = g_FbfTimPath;
            break;
        case FBFD:
             szFileName = g_FbfFilePath;
        break;      
        default:
            dprintf("which file to download?");
            szFileName = NULL;
            break;
    }

    if (!szFileName)
        return -1;

    lFileSize = BinFileWtp_GetFileSize(me, szFileName);
    if ((ImageType == TIMH) || (ImageType == PART))
    {
        //fseek (hFile,0L,SEEK_SET); // Set position to SOF
        BinFileWtp_FseekBin(me,szFileName, 0L, SEEK_SET);
        uiRemainingBytes = (unsigned int)lFileSize;
    }
    else
    {
        //fseek (hFile,4L,SEEK_SET); // Set position to SOF
        BinFileWtp_FseekBin(me,szFileName, 4L, SEEK_SET);
        uiRemainingBytes = (unsigned int)lFileSize - 4;
    }

    VerifyImage (me, pWtpStatus, ACK);

    uiBufferSize = 0;
    DataHeader(me, pWtpStatus, uiRemainingBytes, &uiBufferSize);

    dprintf("\tFlags: %d, BufferSize is 0x%X\n", pWtpStatus->Flags,uiBufferSize);

    pSendDataBuffer = (BYTE*)malloc(sizeof(BYTE)*uiBufferSize);
    while (uiRemainingBytes > 0)
    {
        if(uiBufferSize > uiRemainingBytes)
        {
            uiBufferSize = uiRemainingBytes;
        }
        uiRemainingBytes -= uiBufferSize; // Calculate remaining bytes

        size_t BytesRead = BinFileWtp_ReadBinFile(me,pSendDataBuffer, 1, uiBufferSize, szFileName);

        if (!Data(me, pWtpStatus, pSendDataBuffer, BytesRead, uiRemainingBytes == 0)) return -1;
    }
    free(pSendDataBuffer);

    if (!Done(me, pWtpStatus)) return -1;

    return 0;
}

int WtptpDownLoad_GetDeviceBootType(struct CBinFileWtp *me)
{
    int i = 0;
    WTPSTATUS *pWtpStatus = &gWtpStatus;
    static unsigned int new_size = 0;
    static unsigned int total_size = 0;

    while (i++ < 2) {
        me->m_iSequence = 0;

        SendPreamble(me, pWtpStatus);

        GetWtpMessage(me, pWtpStatus);

        GetVersion(me, pWtpStatus);

        dprintf("\tVersion: %c%c%c%c\n", pWtpStatus->Data[3],pWtpStatus->Data[2],pWtpStatus->Data[1],pWtpStatus->Data[0]);
        dprintf("\tDate: %08X\n",le32toh(*(uint32_t *)&pWtpStatus->Data[4]));
        dprintf("\tProcessor: %c%c%c%c\n",pWtpStatus->Data[11],pWtpStatus->Data[10],pWtpStatus->Data[9],pWtpStatus->Data[8]);   

        if (WtptpDownLoad_DownloadImage(me, pWtpStatus)) return -1;
    }
    
    total_size = 256*1024;    
    update_transfer_bytes(total_size);
    while (1) {
        if (!GetWtpMessage (me, pWtpStatus))
        {
            dprintf ("GetWtpMessage failed before burning flash\n");
            return -1;
        }

        new_size += (32*1024); //12MB/120s
        if (new_size >= (256*1024)) {
            update_transfer_bytes(new_size);
            total_size += new_size;
            new_size = 0;
       }

        if (update_size) {
            if (update_size > total_size) {
                update_transfer_bytes(update_size - total_size);
                total_size = 0;
            }

            update_size = 0;
        }

        if (pWtpStatus->Data[0] == WTPTPNOTIFICATION) {
            uint32_t ReturnCode = le32toh(*(uint32_t *)&pWtpStatus->Data[2]);
            if (ReturnCode == PlatformReady || ReturnCode == PlatformDisconnect) {
                break;
            }
        }

        if ((pWtpStatus->Flags & 0x1 ) != MESSAGEPENDING )
            usleep(500*1000);
    }

    Disconnect(me, pWtpStatus);

   return 0;
}
