#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>

class WinWave
{
    public:
        WinWave();
        ~WinWave();

    public:
        HRESULT LoadWaveFile( const TCHAR* strFileName );

    private:
        HRESULT OpenFile( const TCHAR* strFileName, HMMIO* phmmioIn, MMCKINFO* pckInRIFF );
        HRESULT ReadMMIO( HMMIO hmmioIn, MMCKINFO* pckInRIFF);
        HRESULT StartDataRead( HMMIO* phmmioIn, MMCKINFO* pckIn, MMCKINFO* pckInRIFF );
        HRESULT ReadFile( HMMIO hmmioIn, UINT cbRead, BYTE* pbDest);

    public:
        WAVEFORMATEX*   WaveFormat()    { return pwfx; }
        BYTE*           WaveData()      { return pbData; }
        UINT            WaveSize()      { return cbSize; }

    protected:
        WAVEFORMATEX*   pwfx;
        BYTE*           pbData;
        UINT            cbSize;

};
