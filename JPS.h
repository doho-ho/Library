#pragma once

struct mapNode
{
	int xPos;
	int yPos;
};

struct searchResult
{
	int count;
	mapNode data[dfPATH_POINT_MAX];
};

class BrezenHam
{
private:
	std::list<mapNode*> bhList;

public:
	BrezenHam();
	~BrezenHam();

	bool searchBH(int _sXpos, int _sYpos, int _eXpos, int _eYpos);
	mapNode* getNextPos();
	void listClear();
};

class JumpPointSearch
{
private:

	struct node
	{
		int xPos;
		int yPos;

		float H;
		float G;

		node *parent;
	};

	enum
	{
		LL, LU, LD, UU, RR, RU, RD, DD,
	};

	int maxSize;
	bool **map;			// 동적생성으로 갈겁니다. 

	std::multimap<float, node*> openList;
	std::list<node*> closeList;

	BrezenHam bh;

	int height;
	int width;

	node startNode, endNode, goalNode;

	bool bhTryFlag;
	bool bhResFlag;

	unsigned short openCount, jumpCount;

private:
	void openListClear();
	void closeListClear();

	bool checkDirection(int _xPos, int _yPos, int _direction, node *_parent);
	bool jump(int _width, int _height, int _direction, int *_destWidth, int *_destHeight);
	bool JPS();

	searchResult *getResult(void);
	bool setBHResult(void);

public:
	JumpPointSearch(int _width, int _height, int _max);
	~JumpPointSearch();

	void setMapWall(int _xPos, int _yPos, bool _wall = true);		// 그 부분의 맵에 장애물이 없다고 설정
	
	searchResult* startJPS(int _sXpos, int _sYpos, int _eXpos, int _eYpos);
	
};
