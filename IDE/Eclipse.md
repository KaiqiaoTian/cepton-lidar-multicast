# Eclipse IDE

## Install Eclipse (ROS Melodic) (Ubuntu 18.04)

1. Download the latest "Eclipse IDE for C/C++ Developers" for Linux 64-bit: https://www.eclipse.org/downloads/packages/
1. Extract Eclipse to the desktop.
1. Install Java: `$ sudo apt install openjdk-8-jre`
1. Install gnome-panel `$ sudo apt install gnome-panel`
1. Create launcher for Eclipse on the desktop `$ gnome-desktop-item-edit ~/Desktop/ --create-new`
1. Launcher options
    1. Type: Application
    1. Name: Eclipse
    1. Command: `bash -i -c "~/Desktop/eclipse/eclipse -vmargs -Xmx2048m"`
    1. Icon: Browse for icon.xpm in the eclipse folder
1. Run the Eclipse shortcut on the desktop.

## Install Eclipse (ROS Kinetic) (Ubuntu 16.04)

1. Download Eclipse Oxygen for Linux 64-bit: [eclipse-cpp-oxygen-3-linux-gtk-x86_64.tar.gz](http://www.eclipse.org/downloads/download.php?file=/technology/epp/downloads/release/oxygen/3/eclipse-cpp-oxygen-3-linux-gtk-x86_64.tar.gz)
1. Extract Eclipse to the desktop.
1. Install Java: `$ sudo apt install openjdk-8-jre`
1. Install gnome-panel `$ sudo apt install gnome-panel`
1. Create launcher for Eclipse on the desktop `$ gnome-desktop-item-edit ~/Desktop/ --create-new`
1. Launcher options
    1. Type: Application
    1. Name: Eclipse
    1. Command: `bash -i -c "~/Desktop/eclipse/eclipse -vmargs -Xmx2048m"`
    1. Icon: Browse for icon.xpm in the eclipse folder
1. Run the Eclipse shortcut on the desktop.

## Install Eclipse (ROS Indigo) (Ubuntu 14.04)

1. Download Eclipse Neon for Linux 64-bit: [eclipse-cpp-neon-3-linux-gtk-x86_64.tar.gz](http://www.eclipse.org/downloads/download.php?file=/technology/epp/downloads/release/neon/3/eclipse-cpp-neon-3-linux-gtk-x86_64.tar.gz)
1. Extract Eclipse to the desktop.
1. Install Java: `$ sudo apt-get install openjdk-7-jre`
1. Install gnome-panel `$ sudo apt-get install gnome-panel`
1. Create launcher for Eclipse on the desktop `$ gnome-desktop-item-edit ~/Desktop/ --create-new`
1. Launcher options
    1. Type: Application
    1. Name: Eclipse
    1. Command: `bash -i -c "~/Desktop/eclipse/eclipse -vmargs -Xmx2048m"`
    1. Icon: Browse for icon.xpm in the eclipse folder
1. Run the Eclipse shortcut on the desktop.

## Setup workspace formatting

1. Import the C\+\+ formatting template. Window\->Preferences\->C/C\+\+\->CodeStyle\->Import
1. Browse for Eclipse_ROS_format.xml and select OK.

## Create and import projects

1. Generate Eclipse project. Repeat for each desired package.  
`$ roscd some_package`  
`$ cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_ECLIPSE_MAKE_ARGUMENTS=-j8`
1. Open Eclipse and select File\->Import...  
Select General->Existing Projects into Workspace  
Browse for your project's directory  
Don't check "Copy projects into workspace"  
Select projects to add and click finish  
1. Now you should be able to build the workspace.

