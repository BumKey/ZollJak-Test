#include "TextureMgr.h"

TextureMgr::TextureMgr()
{
}

TextureMgr::~TextureMgr()
{
	for(auto it = mTextureSRV.begin(); it != mTextureSRV.end(); ++it)
    {
		ReleaseCOM(it->second);
    }

	mTextureSRV.clear();
}

void TextureMgr::Init(ID3D11Device * device)
{
	md3dDevice = device;
}

void TextureMgr::Delete(ID3D11ShaderResourceView * srv)
{
	for (auto it = mTextureSRV.begin(); it != mTextureSRV.end();)
	{
		if (it->second == srv) {
			ReleaseCOM(it->second);
			it = mTextureSRV.erase(it);
		}
		else
			++it;
	}
}

ID3D11ShaderResourceView* TextureMgr::CreateTexture(std::wstring filename)
{
	ID3D11ShaderResourceView* srv = 0;

	// Does it already exist?
	if( mTextureSRV.find(filename) != mTextureSRV.end() )
	{
		srv = mTextureSRV[filename];
	}
	else
	{
		HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, filename.c_str(), 0, 0, &srv, 0 ));

		mTextureSRV[filename] = srv;
	}

	return srv;
}
 
