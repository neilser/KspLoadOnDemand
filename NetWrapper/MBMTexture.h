#pragma once

using namespace System;

#include "ITexture.h"
#include "BitmapFormat.h"
#include "AssignableData.h"

ref class MBMTexture : IAssignableTexture{

	array<Byte>^ RawData;
public:
	property int Width;
	property int Height;
	property bool IsNormal;

	MBMTexture(array<Byte>^ data, TextureDebugInfo^ tdi):IAssignableTexture(tdi) {
		System::IO::MemoryStream^ ms = nullptr;
		System::IO::BinaryReader^ bs = nullptr;
		int data_offset = 0;
		D3DFORMAT dataFormat;
		try{
			ms = gcnew System::IO::MemoryStream(data);
			bs = gcnew System::IO::BinaryReader(ms);
			bs->ReadString();
			Width = bs->ReadInt32();
			Height = bs->ReadInt32();
			IsNormal = bs->ReadInt32() == 1;
			int bitPerPixel = bs->ReadInt32();
			switch (bitPerPixel){
			case 32:
				dataFormat = D3DFORMAT::D3DFMT_A8R8G8B8;
				break;
			case 24:
				dataFormat = D3DFORMAT::D3DFMT_R8G8B8;
				break;
			}
			data_offset = (int)ms->Position;
			if (data->Length != ((Width * Height * AssignableFormat::GetByteDepthForFormat(dataFormat)) + data_offset))
			{
				throw gcnew FormatException("Invalid data size. Expected " + ((Width * Height * AssignableFormat::GetByteDepthForFormat(dataFormat)) + data_offset) + " byte, got " + data->Length + " byte (both including a " + data_offset + " byte header)");
			}
		}
		finally{
			if (bs != nullptr)
				delete bs;
			if (ms != nullptr)
				delete ms;
		}
		RawData = gcnew array<Byte>(Width * Height * 4);
		pin_ptr<unsigned char> srcPinPtr = &data[data_offset];
		pin_ptr<unsigned char> trgPinPtr = &RawData[0];
		unsigned char* srcPtr = srcPinPtr;
		unsigned char* trgPtr = trgPinPtr;
		int remainingPixelCount = Width * Height;

		switch (dataFormat){
		case D3DFORMAT::D3DFMT_A8R8G8B8:
			while (remainingPixelCount > 0){
				auto r = (*srcPtr);
				srcPtr++;
				auto g = (*srcPtr);
				srcPtr++;
				auto b = (*srcPtr);
				srcPtr++;
				auto a = (*srcPtr);
				srcPtr++;
				(*trgPtr) = b;
				trgPtr++;
				(*trgPtr) = g;
				trgPtr++;
				(*trgPtr) = r;
				trgPtr++;
				(*trgPtr) = a;
				trgPtr++;
				remainingPixelCount--;
			}
			break;
		case D3DFORMAT::D3DFMT_R8G8B8:
			while (remainingPixelCount > 0){
				auto r = (*srcPtr);
				srcPtr++;
				auto g = (*srcPtr);
				srcPtr++;
				auto b = (*srcPtr);
				srcPtr++;
				(*trgPtr) = b;
				trgPtr++;
				(*trgPtr) = g;
				trgPtr++;
				(*trgPtr) = r;
				trgPtr++;
				(*trgPtr) = 255;
				trgPtr++;
				remainingPixelCount--;
			}
			break;
		}
	}


	virtual AssignableFormat^ GetAssignableFormat() override{
		return gcnew AssignableFormat(Width, Height, D3DFORMAT::D3DFMT_A8R8G8B8);
	}
	virtual AssignableData^ GetAssignableData(AssignableFormat ^ fmt, System::Func<AssignableFormat ^, AssignableData ^> ^% delayedCb) override{
		if (fmt != nullptr){
			throw gcnew NotSupportedException("Cannot convert to other formats, atm (requested: " + fmt + ")");
		}
		auto tmp = gcnew ByteArrayAssignableData(GetAssignableFormat(), RawData, Width * 4, 0, DebugInfo);
		tmp->IsUpsideDown = true;
		return tmp;
	}

	BitmapFormat^ ToBitmap(){
		auto bmp = gcnew Bitmap(Width, Height, PixelFormat::Format32bppArgb);
		auto bmpData = bmp->LockBits(Drawing::Rectangle(0, 0, Width, Height), ImageLockMode::WriteOnly, PixelFormat::Format32bppArgb);
		pin_ptr<unsigned char> srcPinPtr = &RawData[0];
		
		int lineSize = Width * 4;
		GPU::CopyBufferInverted(srcPinPtr, lineSize, (unsigned char*)bmpData->Scan0.ToPointer(), bmpData->Stride, lineSize, Height);
		bmp->UnlockBits(bmpData);
		return gcnew BitmapFormat(bmp, IsNormal, DebugInfo->Modify("ToBitmap"));
	}
	static BitmapFormat^ ConvertToBitmap(MBMTexture^ from){
		return from->ToBitmap();
	}
	static MBMTexture^ Recignizer(System::IO::FileInfo^ file, array<Byte>^ data)
	{
		if (file->Extension->ToUpper() == ".MBM")
		{
			return gcnew MBMTexture(data, gcnew TextureDebugInfo(file->FullName));
		}
		return nullptr;
	}
};