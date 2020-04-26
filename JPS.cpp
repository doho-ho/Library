#include "stdafx.h"
#include "JPS.h"

#define dfPATH_JUMP_TILE 100

BrezenHam::BrezenHam()
{

}

BrezenHam::~BrezenHam()
{

}

void BrezenHam::listClear()
{
	std::list<mapNode*>::iterator iter = bhList.begin();
	std::list<mapNode*>::iterator endIter = bhList.end();
	mapNode *node = NULL;
	for (iter; iter != endIter;)
	{
		node = (*iter);
		iter = bhList.erase(iter);
		delete node;
	}
	return;
}

bool BrezenHam::searchBH(int _sXpos, int _sYpos, int _eXpos, int _eYpos)
{
	if (bhList.size() > 0)
		listClear();

	int e = 0;
	bool heightFlag = false;  // false 면 width가 기준 true면 height가 기준
	int width = _eXpos - _sXpos;
	int height = _eYpos - _sYpos;
	int stan = 0, check = 0;
	int oriHeight = _sYpos, oriWidth = _sXpos;
	bool widthSign = true, heightSign = true;
	if (height < 0) heightSign = false;
	if (width < 0) widthSign = false;
	height = abs(height);
	width = abs(width);
	int count = 0;
	if (height - width > 0)
	{
		heightFlag = true;
		stan = width + 1;
		check = height;
	}
	else
	{
		stan = height + 1;
		check = width;
	}

	while (1)
	{
		count++;
		if (oriWidth == _eXpos && oriHeight == _eYpos)
			break;

		e += stan;
		if (e >check)
		{
			e -= check;

			if (heightFlag)
			{
				if (widthSign)
					oriWidth++;
				else
					oriWidth--;
			}
			else
			{
				if (heightSign)
					oriHeight++;
				else
					oriHeight--;
			}
		}
		if (heightFlag)
		{
			if (heightSign)
				oriHeight++;
			else
				oriHeight--;
		}
		else
		{
			if (widthSign)
				oriWidth++;
			else
				oriWidth--;
		}
		mapNode * newNode = new mapNode;
		newNode->xPos = oriWidth;
		newNode->yPos = oriHeight;
		bhList.push_back(newNode);	
	}
	return true;
}

mapNode* BrezenHam::getNextPos()
{
	if (bhList.size() == 0)
		return NULL;
	std::list<mapNode*>::iterator iter = bhList.begin();
	mapNode* node = (*iter);
	bhList.pop_front();
	return node;
}


// 점프 포인터 서치

JumpPointSearch::JumpPointSearch(int _width, int _height, int _max)
{
	maxSize = _max;
	height = _height;
	width = _width;

	map = new bool*[height];
	int count = 0;
	for (count; count < height; count++)
		map[count] = new bool[width];

	int h = 0, w = 0;
	for (h; h < height; h++)
	{
		w = 0;
		for (w; w < width; w++)
			setMapWall(w,h);
	}
}

JumpPointSearch::~JumpPointSearch()
{
	int count = 0;
	for (count; count < height; count++)
		delete[]map[count];
	delete[] map;

	openListClear();
	closeListClear();
}

void JumpPointSearch::openListClear()
{
	std::multimap<float, node*>::iterator iter = openList.begin();
	std::multimap<float, node*>::iterator endIter = openList.end();
	node* data = NULL;
	for (iter; iter != endIter; )
	{
		data = iter->second;
		iter = openList.erase(iter);
		delete data;
	}
	return;
}

void JumpPointSearch::closeListClear()
{
	std::list<node*>::iterator iter = closeList.begin();
	std::list<node*>::iterator endIter = closeList.end();
	node* data = NULL;
	for (iter; iter != endIter; )
	{
		data = (*iter);
		iter = closeList.erase(iter);
		delete data;
	}
	return;
}

bool JumpPointSearch::checkDirection(int _xPos, int _yPos, int _direction, node *_parent)
{
	int _destXpos = 0, _destYpos = 0;

	if (jump(_xPos, _yPos, _direction, &_destXpos, &_destYpos))
	{
		jumpCount = 0;
		std::list<node*>::iterator closeIter;

		for (closeIter = closeList.begin(); closeIter != closeList.end(); closeIter++)
		{
			if ((*closeIter)->xPos == _destXpos && (*closeIter)->yPos == _destYpos)
				return true;
		}

		node *newNode = new node;
		newNode->xPos = _destXpos;
		newNode->yPos = _destYpos;
		newNode->parent = _parent;
		newNode->H = abs(_destXpos - endNode.xPos) - abs(_destYpos - endNode.yPos);
		newNode->G = _parent->G + abs(_destXpos - _parent->xPos) + abs(_destYpos - _parent->yPos);

		std::multimap<float, node*>::iterator openIter;

		for (openIter = openList.begin(); openIter != openList.end(); openIter++)
		{
			if (openIter->second->xPos == _destXpos && openIter->second->yPos == _destYpos)
			{
				openList.erase(openIter);
				break;
			}
		}

		openList.insert(std::pair<float, node*>(newNode->G + newNode->H, newNode));
		return true;
	}
	jumpCount = 0;
	return false;
}

bool JumpPointSearch::jump(int _xPos, int _yPos, int _direction, int *_destXpos, int *_destYpos)
{
	if (_xPos < 0 || _xPos >= width) return false;
	if (_yPos < 0 || _yPos >= height) return false;
	if (map[_yPos][_xPos]) return false;

	jumpCount++;
	if (jumpCount >= dfPATH_JUMP_TILE)
		return false;

	if (endNode.xPos == _xPos && endNode.yPos == _yPos)
	{
		*_destXpos = _xPos;
		*_destYpos = _yPos;
		return true;
	}

	switch (_direction)
	{
	case LL:
		if (_yPos - 1 >= 0 && (_xPos - 1) >= 0 && _yPos + 1 < height && _xPos + 1 < width)
		{
			if (map[_yPos + 1][_xPos] && !map[_yPos + 1][_xPos - 1])	// 아래가 막혀있고, 앞이 열린 경우
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
			if (map[_yPos - 1][_xPos] && !map[_yPos - 1][_xPos - 1])
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
		}
		_xPos -= 1;
		break;
	case LU:
		if (jump(_xPos, _yPos, LL, _destXpos, _destYpos) || jump(_xPos, _yPos, UU, _destXpos, _destYpos))
		{
			*_destXpos = _xPos;
			*_destYpos = _yPos;
			return true;
		}
		_xPos -= 1;
		_yPos -= 1;
		break;
	case LD:
		if (jump(_xPos, _yPos, LL, _destXpos, _destYpos) || jump(_xPos, _yPos, DD, _destXpos, _destYpos))
		{
			*_destXpos = _xPos;
			*_destYpos = _yPos;
			return true;
		}
		_xPos -= 1;
		_yPos += 1;
		break;
	case UU:
		if (_yPos - 1 >= 0 && _xPos - 1 >= 0 && _yPos + 1 < height && _xPos + 1 < width)
		{
			if (map[_yPos][_xPos - 1] && !map[_yPos - 1][_xPos - 1])
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
			if (map[_yPos][_xPos + 1] && !map[_yPos - 1][_xPos + 1])
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
		}
		_yPos -= 1;
		break;
	case RR:
		if (_yPos - 1 >= 0 && (_xPos - 1) >= 0 && _yPos + 1 < height && _xPos + 1 < width)
		{
			if (map[_yPos + 1][_xPos] && !map[_yPos + 1][_xPos + 1])
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
			if (map[_yPos - 1][_xPos] && !map[_yPos - 1][_xPos + 1])
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
		}
		_xPos += 1;
		break;
	case RU:
		if (jump(_xPos, _yPos, RR, _destXpos, _destYpos) || jump(_xPos, _yPos, UU, _destXpos, _destYpos))
		{
			*_destXpos = _xPos;
			*_destYpos = _yPos;
			return true;
		}
		_xPos += 1;
		_yPos -= 1;
		break;
	case RD:
		if (jump(_xPos, _yPos, RR, _destXpos, _destYpos) || jump(_xPos, _yPos, DD, _destXpos, _destYpos))
		{
			*_destXpos = _xPos;
			*_destYpos = _yPos;
			return true;
		}
		_xPos += 1;
		_yPos += 1;
		break;
	case DD:
		if (_yPos - 1 >= 0 && (_xPos - 1) >= 0 && _yPos + 1 < height && _xPos + 1 < width)
		{
			if (map[_yPos][_xPos - 1] && !map[_yPos + 1][_xPos - 1])
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
			if (map[_yPos][_xPos + 1] && !map[_yPos + 1][_xPos + 1])
			{
				*_destXpos = _xPos;
				*_destYpos = _yPos;
				return true;
			}
		}
		_yPos += 1;
		break;
	default:
		break;
	}
	jumpCount = 0;
	return jump(_xPos, _yPos, _direction, _destXpos, _destYpos);

}

void JumpPointSearch::setMapWall(int _xPos, int _yPos, bool _wall)
{
	map[_yPos][_xPos] = _wall;
	return;
}

searchResult* JumpPointSearch::startJPS(int _sXpos, int _sYpos, int _eXpos, int _eYpos)
{
	startNode.xPos = _sXpos;
	startNode.yPos = _sYpos;
	endNode.xPos = _eXpos;
	endNode.yPos = _eYpos;

	searchResult *result = NULL;

	bhTryFlag = false;
	bhResFlag = false;
	int count = 0;

	node *newNode = new node;
	newNode->G = startNode.G;
	newNode->H = startNode.H;
	newNode->xPos = startNode.xPos;
	newNode->yPos = startNode.yPos;

	openList.insert(std::pair<float, node*>(newNode->G + newNode->H, newNode));

	while (count < dfPATH_JUMP_MAX)
	{
		count++;
		if (JPS())
		{
			result = getResult();
			break;
		}
	}
	
	bhTryFlag = false;
	openListClear();
	closeListClear();

	return result;
}

bool JumpPointSearch::JPS()
{
	if (!bhTryFlag)
	{
		bhTryFlag = true;
		if (bh.searchBH(startNode.xPos, startNode.yPos, endNode.xPos, endNode.yPos) && setBHResult())
		{
			bhResFlag = true;
			return true;
		}
	}

	node *_node;
	std::multimap<float, node*> ::iterator iter;
	iter = openList.begin();
	if (iter == openList.end())
		return false;
	_node = iter->second;
	closeList.push_back(_node);
	openList.erase(iter);


	if (_node->xPos == endNode.xPos && _node->yPos == endNode.yPos)
	{
		goalNode = *_node;
		return true;
	}
	

	if (_node->xPos == startNode.xPos && _node->yPos == startNode.yPos)   // 노드가 시작점 인 경우 (8방향 모두 체크)
	{
		checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
		checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
		checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
		checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
		checkDirection(_node->xPos - 1, _node->yPos - 1, LU, _node);
		checkDirection(_node->xPos + 1, _node->yPos - 1, RU, _node);
		checkDirection(_node->xPos + 1, _node->yPos + 1, RD, _node);
		checkDirection(_node->xPos - 1, _node->yPos + 1, LD, _node);
	}
	else //노드가 시작점이 아닌 경우
	{
		if (!_node->parent) return false;		// 노드가 부모인 경우 있어서는 안되는 경우.

												// 1. 부모의 좌표를 이용해 방향 획득.
		int dx = _node->xPos - _node->parent->xPos;
		int dy = _node->yPos - _node->parent->yPos;
		int direction;

		if (dx < 0 && dy == 0)	 direction = LL;
		if (dx < 0 && dy < 0)		direction = LU;
		if (dx < 0 && dy > 0)		direction = LD;
		if (dx == 0 && dy < 0)	direction = UU;
		if (dx == 0 && dy > 0)	direction = DD;
		if (dx > 0 && dy == 0)	direction = RR;
		if (dx > 0 && dy < 0)		direction = RU;
		if (dx > 0 && dy > 0)		direction = RD;

		switch (direction)
		{
		case LL:
			checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
			if (map[_node->yPos + 1][_node->xPos] && !map[_node->yPos + 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos - 1, _node->yPos + 1, LD, _node);
			}
			if (map[_node->yPos - 1][_node->xPos] && !map[_node->yPos - 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos - 1, _node->yPos - 1, LU, _node);
			}
			break;
		case LU:
			checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
			checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
			checkDirection(_node->xPos - 1, _node->yPos - 1, LU, _node);
			if (map[_node->yPos][_node->xPos + 1] && !map[_node->yPos - 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos + 1, _node->yPos - 1, RU, _node);
			}
			if (map[_node->yPos + 1][_node->xPos] && !map[_node->yPos + 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos - 1, _node->yPos + 1, LD, _node);
			}
			break;
		case LD:
			checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
			checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
			checkDirection(_node->xPos - 1, _node->yPos + 1, LD, _node);
			if (map[_node->yPos][_node->xPos + 1] && !map[_node->yPos + 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos + 1, _node->yPos + 1, RD, _node);
			}
			if (map[_node->yPos][_node->xPos - 1] && !map[_node->yPos - 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos - 1, _node->yPos - 1, LU, _node);
			}
			break;
		case UU:
			checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
			if (map[_node->yPos][_node->xPos + 1] && !map[_node->yPos - 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos + 1, _node->yPos - 1, RU, _node);
			}
			if (map[_node->yPos][_node->xPos - 1] && !map[_node->yPos - 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos - 1, _node->yPos - 1, LU, _node);
			}
			break;
		case DD:
			checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
			if (map[_node->yPos][_node->xPos + 1] && !map[_node->yPos + 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos + 1, _node->yPos + 1, RD, _node);
			}
			if (map[_node->yPos][_node->xPos - 1] && !map[_node->yPos + 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos - 1, _node->yPos + 1, LD, _node);
			}
			break;
		case RR:
			checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
			if (map[_node->yPos + 1][_node->xPos] && !map[_node->yPos + 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos + 1, _node->yPos + 1, RD, _node);
			}
			if (map[_node->yPos - 1][_node->xPos] && !map[_node->yPos - 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos + 1, _node->yPos - 1, RU, _node);
			}
			break;
		case RU:
			checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
			checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
			checkDirection(_node->xPos + 1, _node->yPos - 1, RU, _node);
			if (map[_node->yPos + 1][_node->xPos] && !map[_node->yPos + 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos + 1, _node->yPos + 1, RD, _node);
			}
			if (map[_node->yPos - 1][_node->xPos] && !map[_node->yPos - 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos - 1, _node->yPos - 1, LU, _node);
			}
			break;
		case RD:
			checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
			checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
			checkDirection(_node->xPos + 1, _node->yPos + 1, RD, _node);
			if (map[_node->yPos][_node->xPos - 1] && !map[_node->yPos + 1][_node->xPos - 1])
			{
				checkDirection(_node->xPos - 1, _node->yPos, LL, _node);
				checkDirection(_node->xPos, _node->yPos + 1, DD, _node);
				checkDirection(_node->xPos - 1, _node->yPos + 1, LD, _node);
			}
			if (map[_node->yPos - 1][_node->xPos] && !map[_node->yPos - 1][_node->xPos + 1])
			{
				checkDirection(_node->xPos + 1, _node->yPos, RR, _node);
				checkDirection(_node->xPos, _node->yPos - 1, UU, _node);
				checkDirection(_node->xPos + 1, _node->yPos - 1, RU, _node);
			}
			break;
		default:
			break;
		}
	}
	return false;
}

searchResult* JumpPointSearch::getResult(void)
{
	searchResult* result = new searchResult;

	int count = 0;
	if (bhResFlag)
	{
		result->data[count].xPos = endNode.xPos;
		result->data[count].yPos = endNode.yPos;
		count++;
		result->count = count;
	}
	else
	{
		node *JPSData = &goalNode;
		while (1)
		{
			if (JPSData->xPos == startNode.xPos && JPSData->yPos == startNode.yPos)
				break;
			JPSData = JPSData->parent;
			count++;
		}
		result->count = count;
		int i = count;
		JPSData = &goalNode;
		for (i; i > 0; i--)
		{
			result->data[(i - 1)].xPos = JPSData->xPos;
			result->data[(i - 1)].yPos = JPSData->yPos;
			JPSData = JPSData->parent;
		}
	}
	bhResFlag = false;
	return result;
}

bool JumpPointSearch::setBHResult()
{
	mapNode *data = NULL;
	int xPos, yPos;
	while (1)
	{
		data = NULL;
		data = bh.getNextPos();
		if (!data)
			break;
		xPos = data->xPos;
		yPos = data->yPos;
		delete data;
		if (map[yPos][xPos])
		{
			bh.listClear();
			return false;
		}

	}
	return true;
}