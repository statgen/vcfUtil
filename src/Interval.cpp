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

#include "Interval.h"

template <class T>
const int Interval<T>::BLACK = 0;
template <class T>
const int Interval<T>::RED = 1;

template <class T>
Interval<T>::Interval(int start, int end, int max, T& value, int deviation, int color, Interval<T>* left, Interval<T>* right):
	start(start), end(end), max(max), deviation(deviation), color(color), parent(NULL) {
	values.push_back(value);

	if (left != NULL) {
		this->left = left;
		left->parent = this;
	}
	else {
		this->left = NULL;
	}

	if (right != NULL) {
		this->right = right;
		right->parent = this;
	}
	else {
		this->right = NULL;
	}
}

template<class T>
Interval<T>::Interval(int start, int end, int max, vector<T>& values, int deviation, int color, Interval<T>* left, Interval<T>* right):
	start(start), end(end), max(max), deviation(deviation), color(color), parent(NULL) {
	this->values.insert(this->values.end(), values.begin(), values.end());

	if (left != NULL) {
		this->left = left;
		left->parent = this;
	}
	else {
		this->left = NULL;
	}

	if (right != NULL) {
		this->right = right;
		right->parent = this;
	}
	else {
		this->right = NULL;
	}
}

template <class T>
Interval<T>::~Interval() {
	parent = NULL;
	left = NULL;
	right = NULL;

	values.clear();
}

template <class T>
Interval<T>* Interval<T>::grandparent() {
	if (parent != NULL) {
		return parent->parent;
	}

	return NULL;
}

template <class T>
Interval<T>* Interval<T>::sibling() {
	if (parent != NULL) {
		if (this == parent->left) {
			return parent->right;
		}
		else {
			return parent->left;
		}
	}

	return NULL;
}

template <class T>
Interval<T>* Interval<T>::uncle() {
	if (parent != NULL) {
		return parent->sibling();
	}

	return NULL;
}

template <class T>
int Interval<T>::compare(Interval<T>& interval) {
	if (start == interval.start) {
		if (end == interval.end) {
			return 0;
		}
		else if (end < interval.end) {
			return -1;
		}
	}
	else if (start < interval.start) {
		return -1;
	}

	return 1;
}

template class Interval<int>;
template class Interval<char*>;
