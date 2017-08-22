UAVcontest_4th
=============

*UAVcontest_4th* 是**第一届“高分”无人飞行器智能感知大赛（原上海交大小型无人机大赛）** 决赛项目二“协同搜索”的裁判评分程序。竞赛详细信息见：
[http://drone.sjtu.edu.cn/contest](http://drone.sjtu.edu.cn/contest)

## 比赛方式：
无人机启动后进入自主飞行模式，通过机载传感器，开启智能感知算法，自动控制无人机(群)自动起飞并进入搜索场地进行二维码搜索。该项目任务是在规定时间内完成对场地内所有的二维码的识别检测。

## 比赛流程
1. 参数队伍可使用1台或者多台无人机，最多不超过4架次进行二维码搜索，各无人机从各自停机坪起飞。
2. 穿过虚拟墙（黄色实线），进入搜索区域；
3. 启动目标搜索，识别场景中的二维码，并将拍摄到的二维码图像以如下格式格式保存：
Result/<所识别的数字>.png
4. 在规定时间内搜索完所有目标,所有飞机自动原地降落。
5. 将Result目录结果复制给裁判。
6. 裁判运行 *UAVcontest_4th* 自动评分软件进行评分。

## 评分原理
1. 所拍摄的二维码图片保存格式为：Result/<所识别的数字>.png（例如：识别的数字为100，则以100.png命名图片保存在Result文件夹里），且文件夹中图片数量不能超过20张（如果超过20张，则由裁判随机抽取20张作为提交结果）。
2. 图片中二维码面积占整张图片面积不小于10%，且二维码面积占二维码最小包络正方形面积不小于70%（Area(二维码)/Area(正方形包络）≥ 70%）。

* 图例1：不合格。原因：二维码面积占整张图片面积10%
<img src="https://github.com/danping/uav_contest/blob/master/doc/contest/p1.png" height="150">

* 图例2：不合格。原因：二维码面积占包括整个二维码最小正方形面积小于75%
<img src="https://github.com/danping/uav_contest/blob/master/doc/contest/p2.png" height="200">

* 图例3：不合格。原因：二维码面积占包括整个二维码最小正方形面积小于75%
<img src="https://github.com/danping/uav_contest/blob/master/doc/contest/p3.png" height="200">

* 图例4：合格
<img src="https://github.com/danping/uav_contest/blob/master/doc/contest/p4.png" height="200">

* 图例5：合格
<img src="https://github.com/danping/uav_contest/blob/master/doc/contest/p5.png" height="200">

## 程序运行方式
1. 下载本程序
```
git clone https://github.com/danping/uav_contest.git
```
2. 编译
```
cd uav_contest
mkdir build
cd build
cmake .. #需要sudo apt-get install cmake-curses-gui
make #可执行文件将生成在'build'的bin子目录下
```
3. 运行
```
#首先将contest_sample_data 复制到 /home/<user_name>/Result
./bin/uav_artag_parser
```

*另外种运行方式是在Qtcreater中直接加载CMakeLists.txt导入工程进行编译运行

## 注:
本程序基于ARToolKitPlus开发，详情请见：
[https://github.com/paroj/artoolkitplus](https://github.com/paroj/artoolkitplus)
