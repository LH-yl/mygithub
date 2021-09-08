print("*******Snaker 2.0*******")
print("************************")
print("*******1.普通模式*******")
print("*******2.困难模式*******")
print("*******3.地狱模式*******")
print("************************")
choose = int(input("请选择游戏模式>>> "))
print("******Snaker color******")
print("************************")
print("********1.green*********")
print("********2.pink**********")
print("********3.white*********")
print("************************")
select = int(input("请选择小蛇颜色>>> "))
# 方块大小[20,20]
import random, sys, time, pygame
from pygame.locals import *
# rgb三基色:red green blue
# 定义颜色
gray_color = pygame.Color(128, 128, 128)
red_color = pygame.Color(255, 0, 0)
white_color = pygame.Color(255, 255, 255)
black_color = pygame.Color(0, 0, 0)
green_color = pygame.Color(0, 255, 0)
blue_color = pygame.Color(0, 0, 255)
pink_color = pygame.Color(248, 197, 183)
random.color = pygame.Color(random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

# gameover函数
def gameover(playSurface):
    # 设置字体样式及大小
    gameoverFont = pygame.font.Font("arial.ttf", 72)
    # 设置字体内容及颜色
    gameoverSurf = gameoverFont.render("Game over", True, gray_color)
    # 建立图像连接
    gameoverRect = gameoverSurf.get_rect()
    # 设置结束内容的位置
    gameoverRect.midtop = (320, 100)
    # 所有的连接起来
    playSurface.blit(gameoverSurf, gameoverRect)
    # 刷新
    pygame.display.flip()
    # 5秒后自动执行下面的退出
    time.sleep(5)
    sys.exit()
    pygame.quit()


def main(choose, select):
    if choose == 1:
        temp = 5
    elif choose == 2:
        temp = 10
    elif choose == 3:
        temp = 20
    else:
        temp = 5
    if select == 1:
        snaker_color = green_color
    elif select == 2:
        snaker_color = pink_color
    elif select == 3:
        snaker_color = white_color
    else:
        snaker_color = green_color
    pygame.init()
    # 速度
    snakespeed = pygame.time.Clock()
    # 设置边界
    playSurface = pygame.display.set_mode((640, 460))
    # 内容提示
    pygame.display.set_caption("Snake 2.0")
    # 设置蛇头
    snakeposition = [100, 100]
    # 设置蛇的身体 
    snakelength = [[100, 100], [80, 100], [60, 100], [40, 100], [20, 100]]
    # 设置豆子
    doudouposition = [300, 300]
    # 设置豆子的数量
    doudou = 1
    # 蛇颜色
    # 方向
    direction = "right"
    # 反向方向
    changeDirection = direction
    while True:
        for Event in pygame.event.get():
            if Event.type == QUIT:
                pygame.quit()
                sys.exit()
            elif Event.type == KEYDOWN:
                # w a s d
                # esc  ↑ ↓ ← →
                if Event.key == K_RIGHT or Event.key == ord("d"):
                    changeDirection = "right"
                if Event.key == K_LEFT or Event.key == ord("a"):
                    changeDirection = "left"
                if Event.key == K_UP or Event.key == ord("w"):
                    changeDirection = "up"
                if Event.key == K_DOWN or Event.key == ord("s"):
                    changeDirection = "down"
                if Event.key == K_ESCAPE:
                    pygame.quit()
                    sys.exit()
        # 判断方向（是不是输入了反方向）
        if changeDirection == "right" and not direction == "left":
            direction = changeDirection
        if changeDirection == "left" and not direction == "right":
            direction = changeDirection
        if changeDirection == "up" and not direction == "down":
            direction = changeDirection
        if changeDirection == "down" and not direction == "up":
            direction = changeDirection
        # 没动过，移动蛇
        if direction == "right":
            snakeposition[0] += 20
        if direction == "left":
            snakeposition[0] -= 20
        if direction == "up":
            snakeposition[1] -= 20
        if direction == "down":
            snakeposition[1] += 20
        snakelength.insert(0, list(snakeposition))
        # 判断豆豆
        if snakeposition[0] == doudouposition[0] and snakeposition[1] == doudouposition[1]:
            doudou = 0
        else:
            snakelength.pop()
        # 如果豆被吃掉，重新出现，位置随机
        if doudou == 0:
            x = random.randrange(1, 32)
            y = random.randrange(1, 23)
            doudouposition = [int(x)*20, int(y)*20]
            doudou = 1
        # 绘制界面显示层
        playSurface.fill(black_color)
        for position in snakelength:
            pygame.draw.rect(playSurface, snaker_color, Rect(position[0], position[1], 20, 20))
            random.color = pygame.Color(random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
            pygame.draw.rect(playSurface, random.color, Rect(doudouposition[0], doudouposition[1], 20, 20))
        # 判断死亡
        if snakeposition[0] > 620 or snakeposition[0] < 0:
            gameover(playSurface)
        if snakeposition[1] > 440 or snakeposition[1] < 0:
            gameover(playSurface)
        pygame.display.flip()
        snakespeed.tick(temp)


main(choose, select)











