#pragma  once
#include "windows.h"
#define IS_A_PE 1

class PEClass
{
	public:
			PEClass();
			~PEClass();
			BOOL					LoadPE(char* ModulePath);					//PE�ļ����ص��ڴ�
			ULONG					Rva2Ra(ULONG Rva);							//Rva ת �ļ�ƫ��
			PIMAGE_SECTION_HEADER	AddSections(char* name ,ULONG size);		//�¼�һ����
			PIMAGE_DOS_HEADER		GetImageBuf();								//��ü��ص��ڴ���׵�ַ	
			PIMAGE_SECTION_HEADER	GetSectionByName(char* name);
			PIMAGE_SECTION_HEADER	GetSectionById(ULONG id);
			ULONG					GetEntry();
			void					SetEntry(ULONG OEP);
			DWORD					PEAlign(ULONG num , ULONG Align);
			void					ReadRaw(ULONG pos , PVOID buf, ULONG size);
			void					WriteRaw(ULONG pos,PVOID buf ,ULONG size);
			ULONG					GetSectionsCount();
			void					restView();								//ˢ�� ӳ��
			ULONG					TestIsPE();								//�����Ƿ��ǺϷ�PE
			ULONG					GetFileSizex();							//��ȡ�ļ���С
			bool					FlushToFile();
			ULONG					GetImageBase();

	private:
			PIMAGE_DOS_HEADER m_Buffer;
			HANDLE			  m_hfile ,m_hmap;
};

PEClass::PEClass()
{
	m_Buffer	= NULL;
	m_hmap		= INVALID_HANDLE_VALUE;
	m_hfile  = INVALID_HANDLE_VALUE;
}
PEClass::~PEClass()
{
	if( m_hfile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hfile);
	if(m_hmap != INVALID_HANDLE_VALUE)
		CloseHandle(m_hmap);
	if(m_Buffer != NULL)
		UnmapViewOfFile(m_Buffer);
}

BOOL PEClass::LoadPE(char* ModulePath)
{
	ULONG size;

	m_hfile = CreateFileA(ModulePath,\
							GENERIC_READ | GENERIC_WRITE,
							0,NULL,OPEN_EXISTING,\
							FILE_ATTRIBUTE_NORMAL,0);
	if(m_hfile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	size = GetFileSize(m_hfile,NULL);

	m_hmap = CreateFileMappingA(m_hfile,NULL,
						PAGE_READWRITE,
						0,size,NULL);
	if(m_hmap == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	m_Buffer = (PIMAGE_DOS_HEADER)MapViewOfFile(m_hmap,\
							FILE_MAP_READ|FILE_MAP_WRITE ,\
							0,0,0);

	if(m_Buffer == NULL)
	{
		return false;
	}

return true;
}


ULONG 	PEClass::GetSectionsCount()
{
	PIMAGE_NT_HEADERS pNtHead;
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);
	return (ULONG)(pNtHead->FileHeader.NumberOfSections);
}

ULONG PEClass::Rva2Ra(ULONG Rva)
{
	PIMAGE_NT_HEADERS pNtHead;
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);
	PIMAGE_SECTION_HEADER pImageSectionHeader = \
			(PIMAGE_SECTION_HEADER)((ULONG)pNtHead + sizeof(IMAGE_NT_HEADERS));

	for ( ULONG u_index =0 ;u_index < GetSectionsCount() ; u_index ++)
	{
		if ( Rva >= pImageSectionHeader[u_index].VirtualAddress && \
			Rva <= ( pImageSectionHeader[u_index].VirtualAddress + \
			pImageSectionHeader[u_index].SizeOfRawData )
			)
			
			return Rva - pImageSectionHeader[u_index].VirtualAddress +  \
			pImageSectionHeader[u_index].PointerToRawData;
	}
	return 0;
}

PIMAGE_DOS_HEADER PEClass::GetImageBuf()
{
	return m_Buffer;
}
PIMAGE_SECTION_HEADER PEClass::GetSectionById(ULONG id)
{
	PIMAGE_NT_HEADERS pNtHead;
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);
	PIMAGE_SECTION_HEADER pImageSectionHeader = \
			(PIMAGE_SECTION_HEADER)((ULONG)pNtHead + sizeof(IMAGE_NT_HEADERS));

	return &pImageSectionHeader[id];
}


PIMAGE_SECTION_HEADER PEClass::GetSectionByName(char* name)
{

	PIMAGE_NT_HEADERS pNtHead;
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);
	PIMAGE_SECTION_HEADER pImageSectionHeader = \
			(PIMAGE_SECTION_HEADER)((ULONG)pNtHead + sizeof(IMAGE_NT_HEADERS));

	
	for(int u_index=0; u_index < GetSectionsCount() ; u_index ++)
	{
	
				
		if( ! strcmp((char*)pImageSectionHeader[u_index].Name ,name) )
		{
			printf("%s  0x%x\n" , (char*)pImageSectionHeader[u_index].Name ,&pImageSectionHeader[u_index]);
			return  &pImageSectionHeader[u_index];
		}
	}
	 return NULL;
}

ULONG PEClass::PEAlign(ULONG num , ULONG Align)
{
	if( num % Align)
	{
		return ( num/Align + 1)*Align ;
	}
	else
		return num;
}

void PEClass::restView()	//���ļ�����ӳ��
{
	
	DWORD  size  ;
//	memset(m_Buffer , 0 , GetFileSizex());
	if( UnmapViewOfFile((PVOID)m_Buffer))
		{
		
			CloseHandle(this->m_hmap);
			m_Buffer = NULL;
			size = GetFileSize(m_hfile , 0);
			m_hmap = CreateFileMappingA(m_hfile,NULL,	\
										PAGE_READWRITE,	\
										0,size,NULL);
			m_Buffer = (PIMAGE_DOS_HEADER)MapViewOfFile(m_hmap,\
											FILE_MAP_READ|FILE_MAP_WRITE,\
												0,0,0);
		}
}


PIMAGE_SECTION_HEADER PEClass::AddSections(char* name,ULONG size)
{
	// �ж��Ƿ���
	ULONG temp;
	PIMAGE_NT_HEADERS pNtHead;
	
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);
	temp = ( (ULONG)pNtHead + sizeof(IMAGE_NT_HEADERS) ) ;
	temp += (GetSectionsCount() + 2) * sizeof(IMAGE_SECTION_HEADER);
	if( (temp - (ULONG)m_Buffer ) > pNtHead->OptionalHeader.SizeOfHeaders)	//�жϽ�ͷ�ܷ����
		return NULL;
	
	
	//
	//�¼�һ�� ��ͷ
	ULONG FileAlign , SectionAlign  , last;
	ULONG Ra,Rva;
	PIMAGE_SECTION_HEADER pImageSectionHeader = \
			(PIMAGE_SECTION_HEADER)((ULONG)pNtHead + sizeof(IMAGE_NT_HEADERS));
	FileAlign = pNtHead->OptionalHeader.FileAlignment;
	SectionAlign = pNtHead->OptionalHeader.SectionAlignment;

	last =  GetSectionsCount() - 1;
	Rva = pImageSectionHeader[last].VirtualAddress +\
						PEAlign( pImageSectionHeader[last].Misc.VirtualSize , SectionAlign);

	Ra = pImageSectionHeader[last].PointerToRawData + \
							PEAlign( pImageSectionHeader[last].SizeOfRawData , SectionAlign);


	memcpy(pImageSectionHeader[last+1].Name,name, strlen(name)+1 );

	pImageSectionHeader[last+1].PointerToRawData = Ra;
	pImageSectionHeader[last+1].VirtualAddress = Rva;
	pImageSectionHeader[last+1].Characteristics = 0x0E00000E0;
	pImageSectionHeader[last+1].Misc.VirtualSize = PEAlign(size , SectionAlign);
	pImageSectionHeader[last+1].SizeOfRawData =   PEAlign(size , FileAlign);


	char* buf = NULL;
	DWORD rd =0;
	buf =  new char[pImageSectionHeader[last+1].SizeOfRawData];
	memset( buf ,0 ,pImageSectionHeader[last+1].SizeOfRawData);
	SetFilePointer(m_hfile,
		pImageSectionHeader[last+1].PointerToRawData ,0 ,FILE_BEGIN);
	if( ! WriteFile(m_hfile ,buf ,pImageSectionHeader[last+1].SizeOfRawData ,&rd,0))
	{
		memset(&pImageSectionHeader[last+1] , 0 , sizeof(IMAGE_SECTION_HEADER));
		return NULL;
	}
	delete[] buf;
	// update date
	pNtHead->OptionalHeader.SizeOfImage += PEAlign(pImageSectionHeader[last+1].Misc.VirtualSize ,SectionAlign);
	pNtHead->FileHeader.NumberOfSections ++ ;


	this->restView();					//���ļ�������ӳ��
	
	 pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);
	 pImageSectionHeader = \
			(PIMAGE_SECTION_HEADER)((ULONG)pNtHead + sizeof(IMAGE_NT_HEADERS));
	return &pImageSectionHeader[last+1];
}

void PEClass::WriteRaw(ULONG pos,PVOID buf ,ULONG size)
{
	memcpy( (PVOID)((ULONG)m_Buffer + pos)	,\
				buf,\
				size);
}
void PEClass::ReadRaw(ULONG pos , PVOID buf, ULONG size)
{
	memcpy( (PVOID)	buf,\
			 (PVOID)((ULONG)m_Buffer + pos)	,\
				size);
}

ULONG PEClass::GetEntry()
{

	PIMAGE_NT_HEADERS pNtHead;	
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);

	return pNtHead->OptionalHeader.AddressOfEntryPoint;
}
void PEClass::SetEntry(ULONG OEP)
{
	PIMAGE_NT_HEADERS pNtHead;	
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);
	pNtHead->OptionalHeader.AddressOfEntryPoint = OEP;
}

ULONG 	PEClass::TestIsPE()
{

	PIMAGE_NT_HEADERS pNtHead;
	long ret;
	
	ret = 0 ; // 0
	do 
	{
		if( 0x10000 >= (ULONG) m_Buffer)
		{
			break;
		}
		ret ++ ; // 1
		
		if( 0 == m_Buffer)
		{
			if( 0xFFFF & (ULONG) m_Buffer)	//�ڴ�û����
			{
				break;
			}
		}
		ret ++; //2
		if( IMAGE_DOS_SIGNATURE != m_Buffer->e_magic )
		{
			break;
		}
		ret ++; // 3
		// nt ͷƫ��Ϊ8�ı���
		if( 7 & (m_Buffer->e_lfanew))
		{
			break;
		}
		ret ++; //4
		if(0x1000 <= m_Buffer->e_lfanew)
		{
			break;
		}
		ret ++; //5
		
		pNtHead = (PIMAGE_NT_HEADERS)((long)m_Buffer->e_lfanew + (long)m_Buffer);
		if (IMAGE_NT_SIGNATURE != pNtHead->Signature)
		{
			break;
		}
		ret ++; //6
		if( 1 > pNtHead->FileHeader.NumberOfSections 
			|| pNtHead->FileHeader.NumberOfSections > 96)
		{
			
			break;
		}
		ret ++ ; // 7
		//������Ч��exe pe�ļ�����
		if(!((0x100&pNtHead->FileHeader.Characteristics)
			&&(0==(0x3000&pNtHead->FileHeader.Characteristics))))
		{
			//����dll�ļ�����
			if(!((0x100&pNtHead->FileHeader.Characteristics)
				&&(0x2000&pNtHead->FileHeader.Characteristics)))
			{
				//���������ļ�����
				if(!(0x1000&pNtHead->
					FileHeader.Characteristics))
				{
					break;
				}
			}
		}
		ret ++ ;  // 8
		// PE 32
		if(0x10B != pNtHead->OptionalHeader.Magic)
		{
			break;
		}
		
		ret ++ ; // 9
		// ӳ��ʱ��ʱ���սڶ���
		if( pNtHead->OptionalHeader.SizeOfImage 	& (pNtHead->OptionalHeader.SectionAlignment - 1))
		{
			
			break;
		}
		
		
		ret ++;	// ret =10
		break;
	}while (0);
	return ret;

}

ULONG PEClass::GetFileSizex()
{

	return GetFileSize(this->m_hfile,0);
}

bool PEClass::FlushToFile()
{
	return ! FlushViewOfFile(m_Buffer , GetFileSizex() );
}
ULONG PEClass::GetImageBase()
{
	PIMAGE_NT_HEADERS pNtHead;	
	pNtHead = (PIMAGE_NT_HEADERS)( (ULONG)m_Buffer + m_Buffer->e_lfanew);

	return pNtHead->OptionalHeader.ImageBase;
}