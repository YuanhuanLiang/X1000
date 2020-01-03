##!/bin/sh
tar -xvf bouncycastle.tar.xz
mkdir classes
javac -cp .:bouncycastle/bouncycastle-bcpkix-host.jar:bouncycastle/bouncycastle-host.jar  SignApk.java -d classes
#jar xvf bouncycastle/bouncycastle-bcpkix-host.jar -C class
cd classes
jar xvf ../bouncycastle/bouncycastle-bcpkix-host.jar
jar xvf ../bouncycastle/bouncycastle-host.jar
cd -
jar cvfm signapk.jar SignApk.mf -C classes .
rm classes -rf
tar -cJvf bouncycastle.tar.xz bouncycastle
rm bouncycastle -rf
