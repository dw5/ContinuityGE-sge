/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "editorDoc.h"
#include "heightmap.h"
#include "editorapi.h"
#include "terrain.h"
#include "editorTypes.h"

#include "render.h"
#include "material.h"

#include "readwriteapi.h"
#include "filespec.h"
#include "filepath.h"
#include "globalobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorDoc
//

IMPLEMENT_DYNCREATE(cEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(cEditorDoc, CDocument)
	//{{AFX_MSG_MAP(cEditorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc construction/destruction

cEditorDoc::cEditorDoc()
 : m_pHeightMap(NULL),
   m_pTerrain(NULL)
{
}

cEditorDoc::~cEditorDoc()
{
   Assert(m_pHeightMap == NULL);
}

BOOL cEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Add reinitialization code here (SDI documents will reuse this document)

   Assert(AccessEditorApp() != NULL);

   SafeRelease(m_pMaterial);

   delete m_pTerrain, m_pTerrain = NULL;

   delete m_pHeightMap, m_pHeightMap = NULL;

   BOOL bResult = FALSE;

   cMapSettings mapSettings;

   if (AccessEditorApp()->GetMapSettings(&mapSettings) == S_OK)
   {
      cHeightMap * pHeightMap = NULL;
      if (mapSettings.GetHeightData() == kHeightData_HeightMap)
      {
         pHeightMap = new cHeightMap(0.25f); // TODO: hard-coded height scale
         if (pHeightMap != NULL)
         {
            if (!pHeightMap->Load(mapSettings.GetHeightMap()))
            {
               delete pHeightMap;
               pHeightMap = NULL;
            }
         }
      }

      m_pHeightMap = pHeightMap;

      cAutoIPtr<IEditorTileSet> pTileSet;

      UseGlobal(EditorTileManager);
      if (pEditorTileManager->GetTileSet(mapSettings.GetTileSet(), &pTileSet) == S_OK)
      {
         pEditorTileManager->SetDefaultTileSet(mapSettings.GetTileSet());
      }

      Assert(m_pTerrain == NULL);
      m_pTerrain = new cTerrain;
      if (m_pTerrain != NULL)
      {
         if (m_pTerrain->Create(mapSettings.GetXDimension(),
                                mapSettings.GetZDimension(),
                                kDefaultStepSize,
                                pTileSet, 0, pHeightMap))
         {
            Assert(!m_pMaterial);
            if (pTileSet->GetMaterial(&m_pMaterial) != S_OK)
            {
            }

            bResult = TRUE;
         }
      }
   }

	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc serialization

void cEditorDoc::Serialize(CArchive& ar)
{
   Assert(!"This method should never be called");
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc diagnostics

#ifdef _DEBUG
void cEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void cEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc operations

const sTerrainVertex * cEditorDoc::GetVertexPointer() const
{
   return (m_pTerrain != NULL) ? m_pTerrain->GetVertexPointer() : NULL;
}

size_t cEditorDoc::GetVertexCount() const
{
   return (m_pTerrain != NULL) ? m_pTerrain->GetVertexCount() : 0;
}

/////////////////////////////////////////////////////////////////////////////
// cEditorDoc commands

BOOL cEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   DeleteContents();
   SetModifiedFlag(); // set modified flag during load

   cAutoIPtr<IReader> pReader(FileCreateReader(cFileSpec(lpszPathName)));
   if (!pReader)
   {
      return FALSE;
   }

   Assert(m_pTerrain == NULL);
   m_pTerrain = new cTerrain;
   if (m_pTerrain == NULL)
   {
      ErrorMsg("Error allocating new terrain object\n");
      return FALSE;
   }
   else
   {
      if (FAILED(m_pTerrain->Read(pReader)))
      {
         ErrorMsg1("Error loading document %s\n", lpszPathName);
         return FALSE;
      }
   }

   SetModifiedFlag(FALSE); // start off as unmodified

   return TRUE;
}

BOOL cEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
   cAutoIPtr<IWriter> pWriter(FileCreateWriter(cFileSpec(lpszPathName)));
   if (!pWriter)
   {
      return FALSE;
   }

   if (m_pTerrain != NULL)
   {
      if (FAILED(m_pTerrain->Write(pWriter)))
      {
         ErrorMsg1("Error saving document %s\n", lpszPathName);
         return FALSE;
      }
   }

   SetModifiedFlag(FALSE); // not modified anymore

   return TRUE;
}

void cEditorDoc::DeleteContents() 
{
   delete m_pHeightMap, m_pHeightMap = NULL;

   delete m_pTerrain, m_pTerrain = NULL;

   SafeRelease(m_pMaterial);

   CDocument::DeleteContents();
}
