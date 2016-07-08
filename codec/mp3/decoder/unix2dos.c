
#include <unistd.h>
#include <stdio.h>
int main()
{
	char buf[1000];
	ssize_t got;
	while((got=read(0, buf, 1000))>0)
	{
		ssize_t end=0;
		ssize_t pos=0;
		for(end=0;end<got;++end)
		{
			if(buf[end] == '\n')
			{
				write(1, buf+pos, end-pos);
				write(1, "\r\n", 2);
				pos=end+1;
			}
		}
		write(1, buf+pos, end-pos);
	}
}
