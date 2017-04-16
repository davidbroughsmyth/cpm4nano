/*
* Binary file uploader to emulator cpm4nano
*
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
*
* Static compilation:
* go build bin2nano.go   --> bin2nano.exe
* 
* Usage:
* bin2nano com file.ext
* com - COM port number (for example, 38)
* file.ext - name and extension of file (for example, FILE.COM)
*
*/

package main

import (
    "log"
    "io/ioutil"
    "fmt"
    "github.com/jacobsa/go-serial/serial"
    "os"
)

func main() {

    	
    var length uint32
    const buf_size = 256
    var i uint32
    var crc uint8;
	
    com := "COM" + os.Args[1]
    fmt.Printf("%s\n", com)		

    filename := os.Args[2]
    fmt.Printf("File: %s\n", filename)		
    data, err := ioutil.ReadFile(filename)
    if err != nil {
    	log.Fatal(err)
    }
    length = uint32(len(data))	
    fmt.Printf("File size: %d\n", length)
    
    options := serial.OpenOptions{
    	PortName: com,
    	BaudRate: 9600,
    	DataBits: 8,
    	StopBits: 1,
    	MinimumReadSize: 1,
    }

    comport, err := serial.Open(options)
    if err != nil {
      log.Fatalf("serial.Open: %v", err)
    }

    defer comport.Close()

    buf := make([]byte, buf_size)
    comport.Read(buf)     
	
    b:=make([]byte, 1)
    b[0] = uint8(length - ((length/256))*256);
    n,err := comport.Write(b)
    if err != nil {
    	log.Fatalf("port.Write: %d", n)
    }
    b[0] = uint8(length/256);
    n,err = comport.Write(b)
    if err != nil {
      	log.Fatalf("port.Write: %d", n)
    }

    crc = 0;
    for i = 0; i < length; i++ {
	b[0] = data[i]
	crc = crc + b[0];
	n, err = comport.Write(b)
	if err != nil {
      		log.Fatalf("port.Write: %d", n)
	}
	fmt.Println(i);
	//ACK wait
	for {
		buf := make([]byte, 1)
		buf[0] = 0xFF
		n,err = comport.Read(buf)	
		if err == nil {
			if n > 0 {
				if buf[0] == 0x06 {			
					break
				}
			}
		}
	}
    }
    //crc sent
    b[0] = crc;
    n,err = comport.Write(b)
    if err != nil {
    	log.Fatalf("port.Write: %d", n)
    }
    fmt.Print("O.K.")
}
