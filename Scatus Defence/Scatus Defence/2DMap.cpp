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
{	printf(" �� ��� \n\n");
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
//printf("\n���� ��ġ %f %f\n",temple_loc.x, temple_loc.y);
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
			printf("������ �ε� ����!!!\n");
		
	}
	
	printf("�� ���� �ε� �� \n");
	
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
				printf("����\n");
			}
		}
		
	
	}

	res = fclose(ifp);
	if (res != 0)
	{
		printf("������ ������ �ʾҽ��ϴ� \n");
	}
	printf("������ �������ϴ� \n");
	/*
	ifstream input(FileName); //�Է½�Ʈ�� ����  
	

	while (!input.eof()) //���̾ƴҶ����� �о��. 
	{

		input.getline(line_data, 50);

		// ��ū����  
		char* verify = strtok(line_data, 0 );  //��ū���� ���ڸ� ����  
		char* lines; //��ū���� ���� ���� ������ ����������  
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
			input.close();//�Է½�Ʈ�� ���� . 
		}
		else if (y > 100) {
			input.close();//�Է½�Ʈ�� ���� . 
		}
	}  //��ū�� .*/  
	
}