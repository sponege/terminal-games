#include <bits/stdc++.h>
#include <termio.h>
#include <unistd.h>
#include <fcntl.h>
//config
#define FLAG '^'
#define MINE '!'
//code
using namespace std;
#define MINE_TABLE vector<vector<Mine_Table_Item>>
struct Mine_Table_Item
{
	bool Is_Mine = false;
	bool Is_Choosen = false;
	bool Is_Marked = false;
};
const string Color[10] = 
{
	"255;255;255",
	"0;0;255",
	"0;127;255",
	"0;255;255",
	"0;255;127",
	"0;255;0",
	"127;255;0",
	"255;255;0",
	"255;127;0",//for flags & square with 8 bomb around
	"255;0;0",//for mines
};
void Build_Mine_Table(MINE_TABLE& Mine_Table,int minecount,int ei,int ej)
{
	srand(time(0));
	int q=0;
	while (q<minecount)
	{
		int i = rand()%(Mine_Table.size()-2)+1;
		int j = rand()%(Mine_Table[i].size()-2)+1;
		if (Mine_Table[i][j].Is_Mine) continue;
		if (i==ei&&j==ej) continue;
		Mine_Table[i][j].Is_Mine = true;
		q++;
	}
}
void Change_Screen(bool Is_Construct)
{
	static struct termios tty;
	if (Is_Construct)
	{
		tcgetattr(STDIN_FILENO, &tty);
		struct termios tty_new  = tty;
		tty_new.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &tty_new);
	}
	else
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &tty);	
	}
}
int Count_Around(MINE_TABLE& Mine_Table,int i,int j)
{
	return Mine_Table[i-1][j-1].Is_Mine+Mine_Table[i][j-1].Is_Mine+Mine_Table[i+1][j-1].Is_Mine
		  +Mine_Table[i-1][j].Is_Mine                             +Mine_Table[i+1][j].Is_Mine
		  +Mine_Table[i-1][j+1].Is_Mine+Mine_Table[i][j+1].Is_Mine+Mine_Table[i+1][j+1].Is_Mine;
}
int How_Did_I_Dig(MINE_TABLE& Mine_Table,int i,int j)
{
	if (i==0||i==Mine_Table.size()-1) return 0;
	if (j==0||j==Mine_Table[i].size()-1) return 0;
	if (Mine_Table[i][j].Is_Choosen||Mine_Table[i][j].Is_Marked) return 0;
	if (Mine_Table[i][j].Is_Mine) return -1;
	Mine_Table[i][j].Is_Choosen = 1;
	int Count = Count_Around(Mine_Table, i, j);
	cout << "\x1b[38;2;" << Color[Count] << 'm' << "\x1b[" << i << ';' << j << 'H' << Count << "\x1b[0m";
	int res = 1;
	if (Count==0) res+=How_Did_I_Dig(Mine_Table,i-1,j-1)+How_Did_I_Dig(Mine_Table,i,j-1)+How_Did_I_Dig(Mine_Table,i+1,j-1)
					  +How_Did_I_Dig(Mine_Table,i-1,j)                                  +How_Did_I_Dig(Mine_Table,i+1,j)
					  +How_Did_I_Dig(Mine_Table,i-1,j+1)+How_Did_I_Dig(Mine_Table,i,j+1)+How_Did_I_Dig(Mine_Table,i+1,j+1); 
	return res;
}
void Print_End(MINE_TABLE Mine_Table)
{
	cout << "\x1b[38;2;" << Color[9] << 'm';
	for (int i=1;i<Mine_Table.size()-1;i++) for (int j=1;j<Mine_Table[i].size()-1;j++) if (Mine_Table[i][j].Is_Mine) if (!Mine_Table[i][j].Is_Marked) cout << "\x1b[" << i << ';' << j << 'H' << MINE;
	cout << "\x1b[0m";
}
void Set_Input(bool UnBuffered)
{
	fcntl(STDIN_FILENO, F_SETFL, (UnBuffered)?O_NONBLOCK:0);
}
int main()
{
	//declare
	int height,width;
	int minecount;
	//draw
	cout << "\x1b[s\x1b[?47h\x1b[2J\x1b[1;1H";
	Change_Screen(true);
	//get screen size
	cout << "\x1b[1000B\x1b[1000C\x1b[6n";
	getchar();getchar();//skip 2 char;
	string s = " ";
	while (s[s.size()-1]!=';') s+=(char)getchar();
	height = stoi(s.substr(1,s.size()-2))-2;
	s = " ";
	while (s[s.size()-1]!='R') s+=(char)getchar();
	width = stoi(s.substr(1,s.size()-2));
	minecount = height * width /10 ;
	//read input
	cout << "\x1b[1;1HChoose prefix:\n"
		 << "1: Easy (10x10, 10 mines)\n"
		 << "2: Medium (20x20, 40 mines)\n"
		 << "3: Hard (20x40, 80 mines)\n"
		 << "4: Insane,autofit (" << height << 'x' << width << ", " << minecount << " mines)\n"
		 << "5: Custom\n";
	Reading:
	switch (getchar())
	{
		case '1':
		{
			height = 10;
			width = 10;
			minecount = 10;
			break;
		}
		case '2':
		{
			height = 20;
			width = 20;
			minecount = 40;
			break;
		}
		case '3':
		{
			height = 20;
			width = 40;
			minecount = 80;
			break;
		}
		case '4':
		{
			break;
		}
		case 'q':
		{
			Set_Input(false);
			Change_Screen(false);
			cout << "\x1b[?47l\x1b[u";
			return 0;
		}
		case '5':
		{
			Change_Screen(false);
			int i=0;
			do 
			{
				if (i>0&&i<=height)
				{
					height = i;
					break;
				}
				cout << "Height: ";
			} while (cin >> i);
			i=0;
			do 
			{
				if (i>0&&i<=width)
				{
					width = i;
					break;
				}
				cout << "Width: ";
			} while (cin >> i);
			i=0;
			do 
			{
				if (i>0&&i<=width*height)
				{
					minecount = i;
					break;
				}
				cout << "Number of mines: ";
			} while (cin >> i);

			Change_Screen(true);
			break;
		}
		default:goto Reading;
	}
	cout << "\x1b[2J\x1b[1;1H\x1b[0m";
	for (int i=0;i<height;i++)
	{
		for (int j=0;j<width;j++) cout << '.';
		cout << '\n';
	}
cout << "Remaining flags: " << minecount  << "\nTime: 0";
	//init board
	int CellCount = height * width - minecount;
	int CellOpened = 0;
	height++;
	width++;
	MINE_TABLE Mine_Table(height+1);
	for (int i=0;i<=height;i++) Mine_Table[i].resize(width+1);
	//game loop varible
	int i=height/2+1;
	int j=width/2+1;
	cout << "\x1b[" << i << ';' << j << 'H';
	bool In_GameLoop = true;
	bool Generated = false;
	int Correct_Flagged = 0;
	int Flag_remain = minecount;
	//time
	int Time = 0;
	int Interval = 0;
	bool Start_Clock = false;
	//game loop
	Set_Input(true);
	while (In_GameLoop)
	{
		usleep(1000);
		if (Start_Clock) Interval++;
		if (Interval==1000) 
		{
			Time++;
			cout << "\x1b[" << height+1 << ";1HTime: " << Time << "\x1b[" << i << ';' << j << 'H';
			Interval = 0;
		}
		switch (getchar())
		{
			case 'w':{if (i>1) i--;break;}
			case 's':{if (i<height-1) i++;break;}
			case 'a':{if (j>1) j--;break;}
			case 'd':{if (j<width-1) j++;break;}
			case 'q':{In_GameLoop = false;break;}
			case 'f':
			{
				Start_Clock = true;
				if (!Generated) 
				{
					Build_Mine_Table(Mine_Table,minecount,i,j);
					Generated = true;
					//Print_End(Mine_Table);
				}
				int temp = How_Did_I_Dig(Mine_Table, i, j);
				if (temp==-1)
				{
					Print_End(Mine_Table);
					In_GameLoop = 0;
					Set_Input(false);
					cout << "\x1b[" << height << ";1H\x1b[0m\x1b[KYou lose!";
					getchar();
				}
				else CellOpened += temp;
				break;
			}
			case 'g':
			{
				Start_Clock = true;
				if (!Mine_Table[i][j].Is_Choosen)
				{
					if (Mine_Table[i][j].Is_Marked)
					{
						Mine_Table[i][j].Is_Marked = false;
						Correct_Flagged-=Mine_Table[i][j].Is_Mine;
						cout << "\x1b[38;2;200;200;200m.";
						Flag_remain++;
					}
					else 
					{
						Mine_Table[i][j].Is_Marked = true;
						Flag_remain--;
						Correct_Flagged+=Mine_Table[i][j].Is_Mine;
						cout << "\x1b[38;2;" << Color[8] << 'm' << FLAG;
					}
					cout << "\x1b[" << height << ";1H\x1b[0m\x1b[KRemaining flags: " << Flag_remain;
				}
				break;
			}
			case -1:continue;
		} 
		cout << "\x1b[" << i << ';' << j << 'H'; 
		if (Correct_Flagged == minecount && CellOpened == CellCount)
		{
			In_GameLoop = 0;
			cout << "\x1b[" << height << ";1H\x1b[0m\x1b[KYou won!";
			Set_Input(false);
			getchar();
		}	
	}
	Change_Screen(false);
	Set_Input(false);
	cout << "\x1b[?47l\x1b[u";
	return 0;
}
