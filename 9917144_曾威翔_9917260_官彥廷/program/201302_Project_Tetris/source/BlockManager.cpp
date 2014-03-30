#include "BlockManager.h"
#include "read_data.h"
#include <Math.h>

BlockManager::BlockManager(SceneManager *a_SceneMgr)
{
	mSceneMgr = a_SceneMgr;
	mCurBlocksNum = 0;
	mCurBlock = NULL;
	mEnd = false;
	mTileX = DATA_READER::getTileX();
	mTileY = DATA_READER::getTileY();
	for(int i=0;i < 512;i++)
		mBlockUsed[i] = false;
	for(int i = 0;i < 20;i++){
		mHaveBlock[i] = new bool*[mTileX];
		for(int j = 0;j < mTileX;j++){
			mHaveBlock[i][j] = new bool[mTileY];
			for(int k = 0;k < mTileY;k++)
				mHaveBlock[i][j][k] = false;
		}
	}
	count_rotate=0.0;
	initial=true;
	setupSound();
	score_plus=0;
	floorComplete_now=false;
	mAIPlanned = true;
	mParticleEnabled = true;
	createNewBlock();
}


void BlockManager::setupSound(void)
{
	mSoundMgr = SoundManager::createManager();

	mSoundMgr->init();
	mSoundMgr->setAudioPath( (char*) "..\\..\\media\\music\\" );
	// Just for testing
	mSoundMgr->loadAudio( "02.wav", &Ex_audioId, false);
	mSoundMgr->loadAudio( "03.wav", &Fl_audioId, false);
}

void BlockManager::setParticleEnabled(bool b)
{
	//for(int i=0;i < 512;i++)
	//	if(mBlockUsed[i])
	//		mBlockArr[i]->setParticleEnable(b);
	mParticleEnabled = b;
}


void BlockManager::update(const Ogre::FrameEvent& evt)
{
	Vector3 position = mCurBlock->getPosition();
	for(int i = 0;i < 512;i++){
		if(mBlockUsed[i]){
			mBlockArr[i]->update(evt);
			mBlockArr[i]->setParticleEnable(mParticleEnabled);
		}
	}
	
	resolveBoundCollision();
	
	bool collision = true;
	int  counter = 0;
	while(collision){
		collision = false;
		collision |= resolveBoundCollision();
		collision |= resolveBlockCollision();
		counter++;
		if(counter > 10)break;
	}

	recordLocation();

	if(floorComplete_now){
		floorComplete_now=false;
		mSoundMgr->playAudio( Ex_audioId, false );
	}
	else{}

	if(count_rotate>1.0){
		mBlockArr[next_b]->rotate(0x0002);
		count_rotate=0.0;
	}
	else{
		count_rotate+=evt.timeSinceLastFrame;
	}
}

bool BlockManager::isEnd()
{
	return mEnd;
}

void BlockManager::recordLocation()
{
	int top = 8;
	Real bound = 1500/mTileX;

	for(int i = 0;i < 20;i++){
		for(int j = 0;j < mTileX;j++){
			for(int k = 0;k < mTileY;k++)
				mHaveBlock[i][j][k] = false;
		}
	}

	for(int i = 0;i < 512;i++){
		if(mBlockUsed[i])
			if(mBlockArr[i] != mCurBlock){
				Vector3* pos = mBlockArr[i]->getAllPosition();
				for(int j = 0;j < 4;j++){
					if(mBlockArr[i]->getExist(j)){
						Vector3 position = pos[j];
						int height = Math::Floor((position.y- 500)/bound); //int height = Math::Floor((position.y + bound/2 /bound);
						int length = Math::Floor((position.x + 750 - 7500)/bound);
						int width  = Math::Floor((position.z + 750 - 7500)/bound);
						mHaveBlock[height][length][width] = true;
						if(height >= top)
							mEnd = true;
					}
				}
			}
	}
	
	for(int i = 0;i < 20;i++){
		bool floorComplete = true;
		for(int j = 0;j < mTileX;j++){
			for(int k = 0;k < mTileY;k++)
				if(!mHaveBlock[i][j][k]){
					floorComplete = false;
					break;
				}
			if(!floorComplete)
				break;
		}
		if(floorComplete){
			clearBlock(i);
			floorComplete_now=true;
			score_plus+=10;
		}
	}
}

int BlockManager::getScore(){
	int a=score_plus;
	if(score_plus>0){
		score_plus = 0;
	}
	return a;
}

void BlockManager::clearBlock(int layer)
{
	Real bound  = 1500/mTileX;
	//Real height = layer * bound + 150;
	for(int i = 0;i < 512;i++)
		if(mBlockUsed[i]){
			Vector3* pos = mBlockArr[i]->getAllPosition();
			for(int j = 0;j < 4;j++){
				Vector3 position = pos[j];
				int height = Math::Floor((position.y - 500)/bound);
				int length = Math::Floor((position.x + 750 - 7500)/bound);
				int width  = Math::Floor((position.z + 750 - 7500)/bound);
				if(height == layer){
				//if(Math::Floor(position.y + 0.5) == height){
					mBlockArr[i]->clear(j);
					if(mBlockArr[i]->isAllClear()){
						mBlockArr[i]->setVisible(false);
						mBlockUsed[i] = false;
						mBlockArr[i]->~Block();
					}
				}
			}
		}
}

void BlockManager::createNewBlock()
{
	int i;
	for(i = 0;i < 512;i++){
		if(i==next_b)
			continue;
		if(!mBlockUsed[i])
			break;
	}
	if(i >= 512)return;

	if(initial){
		mBlockUsed[i] = true;
		mBlockArr[i] = new Block(mSceneMgr);
		Vector3 position = Vector3(7500, (750/mTileX) * 21 + 500, 7500);
		mBlockArr[i]->setPosition(position);
		mBlockArr[i]->setVisible(true);
		mCurBlock = mBlockArr[i];
		for(i = 0;i < 512;i++)
			if(!mBlockUsed[i])
				break;
		initial=false;
	}
	else{
		mBlockUsed[next_b] = true;
		Vector3 position = Vector3(7500, (750/mTileX) * 21 + 500, 7500);
		mBlockArr[next_b]->setPosition(position);
		mCurBlock = mBlockArr[next_b];
		mSoundMgr->playAudio( Fl_audioId, false );
	}

	mBlockArr[i] = new Block(mSceneMgr);
	Vector3 position = Vector3(0,-6980,2780);
	mBlockArr[i]->setPosition(position);
	mBlockArr[i]->setVisible(true);
	mBlockArr[i]->setParticleEnable(mParticleEnabled);

	next_b = i;
	mAIPlanned = false;
}

void BlockManager::moveBlock(int dir)
{
	Vector3 position = mCurBlock->getPosition();
	if(dir == MOVE_UP)
		mCurBlock->translate(Vector3(0,0,-1500/mTileY));
	else if(dir == MOVE_DOWN)
		mCurBlock->translate(Vector3(0,0, 1500/mTileY));
	else if(dir == MOVE_LEFT)
		mCurBlock->translate(Vector3(-1500/mTileX,0,0));
	else if(dir == MOVE_RIGHT)
		mCurBlock->translate(Vector3(1500/mTileX ,0,0));
	else if(dir == MOVE_SPEEDUP){
		projectCurrentBlock();
		return;
	}
	resolveMoveCollision(position);
}

void BlockManager::rotateBlock(int dir)
{
	mCurBlock->rotate(dir);
	resolveRotateCollision(dir);
}

void BlockManager::resolveRotateCollision(int dir)
{
	int bound = 1500/mTileX;
	Vector3* pos = mCurBlock->getAllPosition();
	for(int i = 0;i < 4;i++){
		if(mCurBlock->getExist(i)){
			Vector3 position = pos[i];
			Real x = position.x;
			Real z = position.z;
			if(x > 750 + 7500 || x < -750 + 7500 || z > 750 + 7500 || z < -750 +7500){
				mCurBlock->rotateBack(dir);
				return;
			}
		}
	}
	for(int i = 0;i < 512;i++)
		if(mBlockUsed[i])
			if(mCurBlock != mBlockArr[i]){
				Vector3* pos2 = mBlockArr[i]->getAllPosition();
				for(int j = 0;j < 4;j++){
					if(mCurBlock->getExist(j)){
						Vector3 p1 = pos[j];
						for(int k = 0;k < 4;k++){
							if(mBlockArr[i]->getExist(k)){
								Vector3 p2 = pos2[k];
								Vector3 p = (p1 - p2);
								int d = p.normalise();	
								if(Math::Abs(d) < bound){
									mCurBlock->rotateBack(dir);
									return;
								}
							}
						}
					}
				}
			}
}

void BlockManager::projectCurrentBlock()
{
	Real bound = 1500/mTileX;
	Vector3* pos = mCurBlock->getAllPosition();
	int highest = 0;
	int num = 0;
	for(int i = 0;i < 4;i++){
		if(mCurBlock->getExist(i)){
			Vector3 position = pos[i];
			int height = Math::Floor((position.y - 500)/bound);
			int length = Math::Floor((position.x + 750 - 7500)/bound);
			int width  = Math::Floor((position.z + 750 - 7500)/bound);
			for(;height >= 0;height--)
				if(mHaveBlock[height][length][width] == true)break;
			height++;
			if(height > highest){
				highest = height;
				num = i;
			}
		}
	}

	Vector3 position = pos[num];
	int height = highest;
	int length = Math::Floor((position.x + 750 - 7500)/bound);
	int width  = Math::Floor((position.z + 750 - 7500)/bound);

	Real x = length * bound + (pos[0].x - pos[num].x) - mTileX/2 * bound + 7500;
	Real y = height * bound + (pos[0].y - pos[num].y) + bound/2 + 500;
	Real z = width  * bound + (pos[0].z - pos[num].z) - mTileY/2 * bound + 7500;
	mCurBlock->setPosition(Vector3(x,y,z));
}

void BlockManager::resolveMoveCollision(Vector3& prePosition)
{
	int bound = 1500/mTileX;
	Vector3* pos = mCurBlock->getAllPosition();

	for(int i = 0;i < 4;i++){
		if(mCurBlock->getExist(i)){
			Vector3 position = pos[i];
			Real x = position.x;
			Real z = position.z;
			if(x > 750 + 7500 || x < -750 + 7500 || z > 750 + 7500 || z < -750 + 7500){
				mCurBlock->setPosition(prePosition);
				return;
			}
		}
	}
	for(int i = 0;i < 512;i++)
		if(mBlockUsed[i])
			if(mCurBlock != mBlockArr[i]){
				Vector3* pos2 = mBlockArr[i]->getAllPosition();
				for(int j = 0;j < 4;j++){
					if(mCurBlock->getExist(j)){
						Vector3 p1 = pos[j];
						for(int k = 0;k < 4;k++){
							if(mBlockArr[i]->getExist(k)){
								Vector3 p2 = pos2[k];
								Vector3 p = (p1 - p2);
								int d = p.normalise();	
								if(Math::Abs(d) < bound){
									mCurBlock->setPosition(prePosition);
									return;
								}
							}
						}
					}
				}
			}
}
	
bool BlockManager::resolveBlockCollision()
{
	bool collision = false;
	int bound = 1500/mTileX;
	bool create = false;
	for(int i = 0;i < 512;i++){
		if(mBlockUsed[i]){
			Vector3* pos = mBlockArr[i]->getAllPosition();
			for(int j = i + 1;j < 512;j++)
				if(i != j)
				if(mBlockUsed[j]){
					Vector3* pos2 = mBlockArr[j]->getAllPosition();
					for(int k = 0;k < 4;k++){
						if(mBlockArr[i]->getExist(k)){
							Vector3 p1 = pos[k];
							for(int l = 0;l < 4;l++){
								if(mBlockArr[j]->getExist(l)){
									Vector3 p2 = pos2[l];
									Vector3 p = (p1 - p2);
									int d = p.normalise();;	
									if(Math::Abs(d) < bound && p.y != 0){
										collision = true;
										d = bound - d;
										p.x = 0;
										p.z = 0;
										if(p1.y > p2.y)
											mBlockArr[i]->translate(p * d);
										else
											mBlockArr[j]->translate(-p * d);
										create |= (mBlockArr[i] == mCurBlock || mBlockArr[j] == mCurBlock);
										//pos = mBlockArr[i]->getAllPosition();
										//pos2 = mBlockArr[j]->getAllPosition();
										//p1 = pos[k];
									}
								}
							}
						}
					}
				}
		}
	}
	if(create){
		createNewBlock();
	}
	return collision;
}

bool BlockManager::resolveBoundCollision()
{
	bool collision = false;
	bool create = false;
	SceneNode* node = mSceneMgr->getEntity("GroundEntity")->getParentSceneNode();
	Real limit_y = node->getPosition().y + (1500.0/mTileX)/2;
	for(int i = 0;i < 512;i++)
		if(mBlockUsed[i]){
			Vector3* position = mBlockArr[i]->getAllPosition();
			for(int j = 0;j < 4;j++){
				if(mBlockArr[i]->getExist(j)){
					if(position[j].y < limit_y){
						collision = true;
						position = mBlockArr[i]->getAllPosition();
						position[0].y = limit_y + (position[0].y - position[j].y);// + 500;
						mBlockArr[i]->setPosition(position[0]);
						position = mBlockArr[i]->getAllPosition();
						create |= (mBlockArr[i] == mCurBlock);
					}
					if(mBlockArr[i] == mCurBlock)
						create |= (position[j].y == limit_y);
				}
			}
		}
	if(create)
		createNewBlock();
	return collision;
}

void BlockManager::AIPlan()
{
	if(!mAIPlanned){
		AIPlanLogic();
		mAIPlanned = true;
	}
}

void BlockManager::AIPlanLogic()
{
	Real bound = 1500/mTileX;
	Vector3* pos = mCurBlock->getAllPosition();
	Vector3* shape = pos;

	int height = -1;//highest;
	int length = 0;//Math::Floor((position.x + 750 - 7500)/bound);
	int width  = -1;//Math::Floor((position.z + 750 - 7500)/bound);

	int** heightMap;
	int   min = INT_MAX;
	int   max = INT_MIN;
	heightMap = new int*[mTileX];
	for(int i = 0;i < mTileX;i++)
		heightMap[i] = new int[mTileY];

	for(int i = 0;i < mTileX;i++)
		for(int j = 0;j < mTileY;j++)
			for(int k = 0;k < 20;k++)
				if(!mHaveBlock[k][i][j]){
					heightMap[i][j] = k;
					if(k > max)max = k;
					if(k < min)min = k;
					break;
				}

	/*Slove ALL direction of block and find the target position*/
	bool setSucc = false;
RE_FIND:
	//for(int layer = min;layer < max + 1;layer++)
	for(int a = 0;a < 4;a++){
		for(int b= 0;b < 4;b++){
			for(int c = 0;b < 4;b++){
				Vector3 findShape[3];
				Vector3 lowBound = Vector3(INT_MAX,INT_MAX,INT_MAX);
				for(int i = 0;i < 3;i++){
					pos = mCurBlock->getAllPosition();
					shape = pos;
					findShape[i] = shape[i + 1] - shape[0];
					int x = (int)Math::Floor((findShape[i].x / bound) + 0.5);
					int y = (int)Math::Floor((findShape[i].y / bound) + 0.5);
					int z = (int)Math::Floor((findShape[i].z / bound) + 0.5);
					//printf("( %f , %f , %f ) ( %d , %d , %d )\n",findShape[i].x,findShape[i].y,findShape[i].z,x,y,z);
					findShape[i] = Vector3(x,y,z);
					if(y < lowBound.y)
						lowBound = findShape[i];
				}
				for(int i = 0;i < mTileX;i++){
					for(int j = 0;j < mTileY;j++){
						//if((int)heightMap[i][j] <= layer && !mHaveBlock[(int)heightMap[i][j]][i][j]){// && !mHaveBlock[layer + (int)lowBound.y][i + (int)lowBound.x][j + (int)lowBound.z]){
						if(!mHaveBlock[min][i][j]){
							bool set = true;
							for(int k = 0;k < 3;k++){
								int local_x = i + (int)findShape[k].x;
								int local_y = (int)(heightMap[i][j] - lowBound.y + findShape[k].y + 0.5);
								int local_z = j + (int)findShape[k].z;
								//printf("%d  %d  %d\n",local_x,local_y,local_z);
								if(local_x >= mTileX || local_x < 0 ||
								   local_z >= mTileY || local_z < 0 ||
								   local_y >= 8		 || local_y < 0)
									set = false;
								else{
									for(int l = 0;l < 8;l++){
										/*block Logic*/
										if(mHaveBlock[l][local_x][local_z] == true && l >= local_y)
											set = false;
										else if(mHaveBlock[l][i][j] == true && l >= heightMap[i][j] - lowBound.y)
											set = false;
										else if(mHaveBlock[l][local_x][local_z] == false && l < local_y){//&& !((findShape[k].x == lowBound.x) && (findShape[k].z == lowBound.z)))
											bool set2 = false;
											if(findShape[k].x == 0 && findShape[k].z == 0 && findShape[k].y > 0)
												set2 = true;
											for(int m = 0;m < 3;m++){
												if(m != k && findShape[k].x == findShape[m].x && findShape[k].z == findShape[m].z && findShape[k].y > findShape[m].y)
													set2 = true;
											}
											set &= set2;
										}
										else if(mHaveBlock[l][i][j] == false && l < heightMap[i][j] - lowBound.y){// && !(i == lowBound.x && j == lowBound.z))
											bool set2 = false;
											for(int m = 0;m < 3;m++){
												if(findShape[m].x == 0 && findShape[m].z == 0 && findShape[m].y < 0)
													set2 = true;
											}
											set &= set2;
										}
									}
								}
							}
							if(set){
								length = i;
								width = j;
								setSucc = true;
								break;
							}
						}
					}
					if(setSucc)break;
				}
				if(setSucc)break;
				mCurBlock->rotate(Block::ROTATE_Z);
			}
			if(setSucc)break;
			mCurBlock->rotate(Block::ROTATE_Y);
		}
		if(setSucc)break;
		mCurBlock->rotate(Block::ROTATE_X);
	}
	if(!setSucc){
		min++;
		if(min > max)
			printf("error in set target\n");
		else
			goto RE_FIND;
		//printf("error in set target\n");
	}

	Real x = length * bound - mTileX/2 * bound + 7500;
	Real y = height * bound + bound/2 + 500;
	Real z = width  * bound - mTileY/2 * bound + 7500;
	
	mTargetPosition = Vector3(x,y,z);
	for(int i = 0;i < 3;i++){
		mShape[i] = shape[i + 1] - shape[0];
		int x = Math::Floor(((int)mShape[i].x / bound) + 0.5);
		int y = Math::Floor(((int)mShape[i].y / bound) + 0.5);
		int z = Math::Floor(((int)mShape[i].z / bound) + 0.5);
		mShape[i] = Vector3(x,y,z);
	}
}

int BlockManager::AIMovePlan()
{
	Vector3 pos = mCurBlock->getPosition();
	if(pos.x - mTargetPosition.x > 0)
		return MOVE_LEFT;
	else if (pos.x - mTargetPosition.x < 0)
		return MOVE_RIGHT;
	else if (pos.z - mTargetPosition.z > 0)
		return MOVE_UP;
	else if (pos.z - mTargetPosition.z < 0)
		return MOVE_DOWN;
	else
		return MOVE_NONE;
}

int BlockManager::AIRotatePlan()
{
	Vector3* pos = mCurBlock->getAllPosition();
	Vector3  shape[3];
	Real    bound = 1500 / mTileX;
	for(int i = 0;i < 3;i++){
		shape[i] = pos[i + 1] - pos[0];
		int x = Math::Floor(((int)shape[i].x / bound) + 0.5);
		int y = Math::Floor(((int)shape[i].y / bound) + 0.5);
		int z = Math::Floor(((int)shape[i].z / bound) + 0.5);
		shape[i] = Vector3(x,y,z);
		//printf("( %f , %f , %f)     ( %f , %f , %f) \n",mShape[i].x,mShape[i].y,mShape[i].z,shape[i].x,shape[i].y,shape[i].z);
	}
	if(shape[0] == mShape[0] && shape[1] == mShape[1] && shape[2] == mShape[2])
		return Block::ROTATE_NONE;
	for(int i = 0;i < 3;i++)
		if(shape[i] == mShape[i]){
			if(shape[i].normalisedCopy() == Vector3::UNIT_X || shape[i].normalisedCopy() == Vector3::NEGATIVE_UNIT_X)
				return Block::ROTATE_X;
			if(shape[i].normalisedCopy() == Vector3::UNIT_Y || shape[i].normalisedCopy() == Vector3::NEGATIVE_UNIT_Y)
				return Block::ROTATE_Y;
			if(shape[i].normalisedCopy() == Vector3::UNIT_Z || shape[i].normalisedCopy() == Vector3::NEGATIVE_UNIT_Z)
				return Block::ROTATE_Z;
		}
	bool dir[3];
	for(int i = 0;i < 3;i++)
		dir[i] = true;
	for(int i = 0;i < 3;i++){
		if(shape[i].normalisedCopy() == Vector3::UNIT_X || shape[i].normalisedCopy() == Vector3::NEGATIVE_UNIT_X)dir[0] = false;
		if(shape[i].normalisedCopy() == Vector3::UNIT_Y || shape[i].normalisedCopy() == Vector3::NEGATIVE_UNIT_Y)dir[1] = false;
		if(shape[i].normalisedCopy() == Vector3::UNIT_Z || shape[i].normalisedCopy() == Vector3::NEGATIVE_UNIT_Z)dir[2] = false;
	}
	if(dir[0])
		return Block::ROTATE_X;
	if(dir[1])
		return Block::ROTATE_Y;
	if(dir[2])
		return Block::ROTATE_Z;

	return Block::ROTATE_NONE;
}