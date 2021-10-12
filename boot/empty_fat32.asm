DD 0x0FFFFFF8 ; 坏簇，不存在的文件
DD 0x0FFFFFFF ; 链终点
DD 0x0FFFFFFF ; 根目录表

TIMES 512 - ($$ - $) DB 0x00