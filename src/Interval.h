/*
 * Copyright © 2011 Daniel Taliun, Christian Fuchsberger and Cristian Pattaro. All rights reserved.
 *
 * This file is part of GWAtoolbox.
 *
 * GWAtoolbox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GWAtoolbox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GWAtoolbox.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INTERVAL_H_
#define INTERVAL_H_

#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

template <class T>
class Interval {
public:
	static const int BLACK;
	static const int RED;

	int start;
	int end;
	int max;

	vector<T> values;
	int deviation;

	int color;
	Interval<T>* parent;
	Interval<T>* left;
	Interval<T>* right;

	Interval(int start, int end, int max, T& value, int deviation, int color, Interval<T>* left, Interval<T>* right);
	Interval(int start, int end, int max, vector<T>& values, int deviation, int color, Interval<T>* left, Interval<T>* right);
	virtual ~Interval();
	Interval<T>* grandparent();
	Interval<T>* sibling();
	Interval<T>* uncle();

	int compare(Interval<T>& interval);
};

#endif
