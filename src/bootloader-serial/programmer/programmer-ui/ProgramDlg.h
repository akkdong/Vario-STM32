#pragma once

#include "Packet.h"


//////////////////////////////////////////////////////////////////////////
//

#define STM32F10X_HD

#define ADDRESS_USER_APPLICATION			(0x08003000)
#define ADDRESS_FLASH						(0x08000000)
#define ADDRESS_RAM							(0x20000000)

#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
#define PAGE_SIZE							(0x400)    // 1 KByte
#define FLASH_SIZE							(0x20000)  // 128 KBytes
#elif defined STM32F10X_CL
#define PAGE_SIZE							(0x800)    // 2 KBytes
#define FLASH_SIZE							(0x40000)  // 256 KBytes
#elif defined STM32F10X_HD || defined (STM32F10X_HD_VL)
#define PAGE_SIZE							(0x800)    // 2 KBytes
#define FLASH_SIZE							(0x80000)  // 512 KBytes
#elif defined STM32F10X_XL
#define PAGE_SIZE							(0x800)    // 2 KBytes
#define FLASH_SIZE							(0x100000) // 1 MByte
#else
#error "Please select first the STM32 device to be used (in stm32f10x.h)"
#endif

// Compute the FLASH upload image size
#define FLASH_IMAGE_SIZE					(uint32_t) (FLASH_SIZE - (ADDRESS_USER_APPLICATION - ADDRESS_FLASH))

#define PROGRAM_SIZE						(0x400)	// 256 bytes
#define PROGRAM_COUNT						(PAGE_SIZE / PROGRAM_SIZE)


////////////////////////////////////////////////////////////////////////////////////
//

class CDownloaderDlg;


////////////////////////////////////////////////////////////////////////////////////
//

class CProgramDlg : public CDialogEx, protected PacketListener
{
public:
	CProgramDlg(CDownloaderDlg * pParent);
	~CProgramDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRAM };
#endif

	//
	enum State
	{
		_READY,
		_RUN,
		_ERASE,
		_PROGRAM,
		_VERIFY,
		_DONE
	};

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void		OnPacketReceived(PACKET * pPacket);

	BOOL				UpdateData(BOOL bSaveAndValidate = TRUE);

private:
	void				RequestErase(uint32_t address);
	void				RequestProgram(uint32_t address);
	void				RequestVerify(uint32_t address);
	void				RequestRun();

	BOOL				FileExist(LPCTSTR lpszFile, uint32_t * pSize = NULL);
	BOOL				LoadFile(LPCTSTR lpszFile);

	void				EnableControls();
	void				Cleanup();

protected:
	//
	CString				m_strFilePath;
	BOOL				m_bVerifyProgram;
	BOOL				m_bRunAfterProgram;

	//
	CDownloaderDlg *	m_pDlgMain;

	//
	UINT				m_nTimerID;

	//
	State				m_nState;

	uint8_t	*			m_pFirmware;
	uint32_t			m_nFileSize;		// actual file size
	uint32_t			m_nProgramSize;		// file size + page padding

	uint32_t			m_nProgramStart;
	uint32_t			m_nProgramEnd;

	uint32_t			m_nTotalPage;		// program size / PAGE_SIZE
	uint32_t			m_nActivePage;		// 0 ~ m_nTotalPage
	uint32_t			m_nActiveSubPage;	// 0 ~ (PAGE_SIZE / PROGRAM_SIZE)



protected:
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();

	afx_msg void		OnTimer(UINT nIDEvent);
	afx_msg void		OnBrowse();
	afx_msg void		OnStart();
	afx_msg void		OnExecute();
	afx_msg LRESULT		OnError(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

