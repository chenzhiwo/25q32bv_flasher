# 25q32bv_flasher
这个项目是为了用树莓派修复我固件丢失的路由器，详细的芯片用法可以参考datasheet。这个程序只是简单地把以参数1为文件名的ROM写入芯片中，再以参数2为文件名从芯片中读出数据，此时可以用md5sum等校验数据的正确性。编译前要先安装wiringpi库，目前树莓派最新系统都会预装

 ![image](https://github.com/chenzhiwo/25q32bv_flasher/blob/master/img/25q32bv.jpg)

