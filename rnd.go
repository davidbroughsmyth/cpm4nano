/*
* Copyright (C) 2017 - Alexey V. Voronin @ FoxyLab
* Email:    support@foxylab.com
* Website:  https://acdc.foxylab.com
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

package main

import (
    "fmt"
)

func main() {

    var length uint32
	var x uint16	
	var seed uint16
	var i uint32    	

	seed = 1111
	x = seed
	length = 100

	for i = 0; i < length; i++ {
  		x ^= x << 2
		x ^= x >> 7
		x ^= x << 7
	fmt.Printf("%0000x\n", x)
	}


}

