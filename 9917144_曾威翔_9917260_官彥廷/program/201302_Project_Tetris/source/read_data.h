/*!
\brief Data from outer file,read and store the value.
*/
#ifndef __DATA_READER_H__
#define __DATA_READER_H__

class DATA_READER {
protected:
	static int mTileX;
	static int mTileY;
	static std::string mbackMusic;
public:
	DATA_READER();
	static void readData();
	static int getTileX();
	static int getTileY();
	static std::string getBackMusic();
};

#endif