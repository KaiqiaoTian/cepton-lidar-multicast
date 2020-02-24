# KDevelop IDE
## Install KDevelop
1. Install KDevelop from apt-get `$ sudo apt-get install kdevelop`
2. Create a desktop launcher in the home directory `$ gedit ~/kdevelop.desktop`
3. Paste the following in the launcher file:
```
[Desktop Entry]
Type=Application
Terminal=false
Exec=bash -i -c "kdevelop"
Name=kdevelop
Icon=kdevelop
```
4. Make desktop launcher executable `$ chmod +x ~/kdevelop.desktop`
5. Only use this desktop launcher to run KDevelop.
## Import ROS Workspace CMakeLists
1. In the **Project** menu, click **Open / Import Project**.
2. Navigate to the **CMakeLists.txt** in the **src** folder of a ROS workspace and click **Next**.
3. On the build directory configuration screen, point KDevelop to the **ros/build** folder in the ROS workspace if you want KDevelop to compile into the same folder as **catkin_make**.

