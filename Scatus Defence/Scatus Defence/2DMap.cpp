#include "2DMap.h"
Map::Map()
{
	
}


Map::~Map()
{
}
void Map::Save(char* FileName)
{
	

}
void Map::print()
{	printf(" 맵 출력 \n\n");
/*
for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			for (auto it: m_Walls)
			{
				if (it.m_vPosition.x / 10 == j && it.m_vPosition.y / 10 == i)
				{
					printf("1 ");
				}
				else if(temple_loc.x /10 == j && temple_loc.y/10 == i)
				{
					printf("R ");
				}
				else if (enemy_loc.x / 10 == j && enemy_loc.y / 10 == i)
				{
					printf("E ");
				}
				else {
					printf("0");
				}
			}
			printf("\n");
			
		}
		printf("\n\n");
	}
	*/
//printf("\n신전 위치 %f %f\n",temple_loc.x, temple_loc.y);
}

void Map::Load(char* FileName)
{
	Wall2D temp_wall;
	int rows = 0;
	int cols = 0;
	char line_data[100];
	const double size_cell = 10.0f;
	FILE *ifp, *ofp;
	char ch= NULL;
	int res;	
	int x = 0, y = 0;
	
	ifp = fopen(FileName, "r");
	if (ifp == NULL)
	{
		for (int i = 0; i < 10; i++)
			printf("맵파일 로딩 실패!!!\n");
		
	}
	
	printf("맵 파일 로딩 중 \n");
	
	while (1)
	{
		

		ch = fgetc(ifp);
		if (ch == EOF || ch == 'e' || y==30) break;
		//temp_wall.m_vPosition.x = x* size_cell;
	//	temp_wall.m_vPosition.y = y* size_cell;
		{
			printf("%c", ch); 
			switch (ch)
			{
			case 51: m_Walls.push_back(temp_wall);
				break;
		//	case 52/*map_temple*/: temple_loc = temp_wall.m_vPosition;
		//		break;
		//	case 53: enemy_loc = temp_wall.m_vPosition;
	//			break;
			default:
				break;
			}
			if (ch != 10)
			{
				x++;
			}if(ch == 10)
			{
				x = 0; y++;
				printf("공백\n");
			}
		}
		
	
	}

	res = fclose(ifp);
	if (res != 0)
	{
		printf("파일이 닫히지 않았습니다 \n");
	}
	printf("파일이 닫혔습니다 \n");
	/*
	ifstream input(FileName); //입력스트림 열고  
	

	while (!input.eof()) //끝이아닐때까지 읽어옴. 
	{

		input.getline(line_data, 50);

		// 토큰시작  
		char* verify = strtok(line_data, 0 );  //토큰으로 숫자만 뽑음  
		char* lines; //토큰으로 걸은 값을 저장할 문자포인터  
		while (verify)
		{
			lines = verify;
			verify = strtok(NULL, 0);
			temp_wall.m_vPosition.x = x* size_cell;
			temp_wall.m_vPosition.y = y* size_cell;
			switch ((int)atoi(lines))
			{
			case map_wall: m_Walls.push_back(temp_wall);
				break;
			case map_temple: temple_loc = temp_wall.m_vPosition;
				break;
			case map_enemy: enemy_loc = temp_wall.m_vPosition;
				break;
			default:
				break;
			}
			x++;
		}
		x = 0, y++;
		if (lines == "end") {
			input.close();//입력스트림 닫음 . 
		}
		else if (y > 100) {
			input.close();//입력스트림 닫음 . 
		}
	}  //토큰끝 .*/  
	
}