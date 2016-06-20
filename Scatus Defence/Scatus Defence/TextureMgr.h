#ifndef TEXTUREMGR_H
#define TEXTUREMGR_H

#include "d3dUtil.h"
#include "Singletone.h"
#include <map>

#define Texture_Mgr TextureMgr::GetInstance()

///<summary>
/// Simple texture manager to avoid loading duplicate textures from file.  That can
/// happen, for example, if multiple meshes reference the same texture filename. 
///</summary>
class TextureMgr : public Singletone<TextureMgr>
{
private:
	TextureMgr();
	~TextureMgr();

	friend class Singletone<TextureMgr>;
public:
	void Init(ID3D11Device* device);
	void Delete(ID3D11ShaderResourceView* srv);

	ID3D11ShaderResourceView* CreateTexture(std::wstring filename);

private:
	TextureMgr(const TextureMgr& rhs);
	TextureMgr& operator=(const TextureMgr& rhs);
	
private:
	ID3D11Device* md3dDevice;
	std::map<std::wstring, ID3D11ShaderResourceView*> mTextureSRV;
};

#endif // TEXTUREMGR_H