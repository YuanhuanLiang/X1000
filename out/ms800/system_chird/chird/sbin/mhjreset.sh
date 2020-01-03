#!/bin/sh

i=0
while [ 1 ];
do
	if [ $(( $i % 2 )) = 0 ]; then
                pwm 4 80000 90
        else
                pwm 4 80000 0
        fi

        let "i=i+1"
	if [ $i = 10 ]; then
                 exit
        fi

        usleep 50000
done

exit 0
