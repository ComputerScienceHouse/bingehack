#ifndef clusterizer_H
#define clusterizer_H

#include <qrect.h>

class Clusterizer {
public:
	Clusterizer(int maxclusters);
	~Clusterizer();

	void add(int x, int y); // 1x1 rectangle (point)
	void add(int x, int y, int w, int h);
	void add(const QRect& rect);

	void clear();
	int clusters() { return count; }
	const QRect& operator[](int i);

private:
	QRect* cluster;
	int count;
	const int max;
};

#endif
