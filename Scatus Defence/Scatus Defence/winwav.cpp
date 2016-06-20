#include "winwav.h"

WinWave::WinWave()
 : pbData( NULL ),
   pwfx( NULL ),
   cbSize( 0 )
{
}

WinWave::~WinWave()
{
    if( pbData != NULL )
    {
        delete [] pbData;
        pbData = NULL;
    }

    if( pwfx != NULL )
    {
        delete pwfx;
        pwfx = NULL;
    }
}

HRESULT WinWave::LoadWaveFile( const TCHAR* strFilename )
{
    HMMIO       hmmioIn;
    MMCKINFO    ckInRiff;
    MMCKINFO    ckIn;

    if( FAILED( OpenFile( strFilename, &hmmioIn, &ckInRiff ) ) )
        return E_FAIL;

    if( SUCCEEDED( StartDataRead( &hmmioIn, &ckIn, &ckInRiff ) ) )
    {
        if( ( pbData = new BYTE[ckIn.cksize] ) )
        {
            if( SUCCEEDED( ReadFile( hmmioIn, ckIn.cksize, pbData) ) )
            {
                mmioClose( hmmioIn, 0 );
                return S_OK;
            }
        }
    }

    mmioClose( hmmioIn, 0 );

    delete pwfx;
    pwfx = NULL;

    return E_FAIL;
}

HRESULT WinWave::OpenFile( const TCHAR* strFileName, HMMIO* phmmioIn, MMCKINFO* pckInRIFF )
{
    HRESULT hr;
    HMMIO   hmmioIn = NULL;

    if( NULL == ( hmmioIn = mmioOpen( (TCHAR*)strFileName, NULL, MMIO_ALLOCBUF|MMIO_READ ) ) )
        return E_FAIL;


    if( FAILED( hr = ReadMMIO( hmmioIn, pckInRIFF) ) )
    {
        mmioClose( hmmioIn, 0 );
        return E_FAIL;
    }

    *phmmioIn = hmmioIn;
    return S_OK;
}

HRESULT WinWave::ReadMMIO( HMMIO hmmioIn, MMCKINFO* pckInRIFF)
{
    MMCKINFO        ckIn;
    PCMWAVEFORMAT   pcmWaveFormat;

    if ( pwfx != NULL )
    {
        delete pwfx;
    }

    pwfx = NULL;

    pckInRIFF->fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if( ( 0 != mmioDescend( hmmioIn, pckInRIFF, NULL, MMIO_FINDRIFF ) ) )
        return E_FAIL;

    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' '); //4개 문자여야 하므로 공백을 추가
    if( 0 != mmioDescend(hmmioIn, &ckIn, pckInRIFF, MMIO_FINDCHUNK) )
        return E_FAIL;

    if( mmioRead(hmmioIn, (HPSTR) &pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
        return E_FAIL;

    pwfx = new WAVEFORMATEX;


    memcpy( pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
    pwfx->cbSize = 0;

    mmioAscend(hmmioIn, &ckIn, 0);
    return S_OK;
}

HRESULT WinWave::StartDataRead( HMMIO* phmmioIn, MMCKINFO* pckIn, MMCKINFO* pckInRIFF )
{
    pckIn->ckid = mmioFOURCC('d', 'a', 't', 'a');

    if( 0 != mmioDescend( *phmmioIn, pckIn, pckInRIFF, MMIO_FINDCHUNK ) )
          return E_FAIL;

    return S_OK;
}

HRESULT WinWave::ReadFile( HMMIO hmmioIn, UINT cbRead, BYTE* pbDest)
{
    mmioRead(hmmioIn, (LPSTR)pbDest, cbRead);
    cbSize = cbRead;
    return S_OK;
}
