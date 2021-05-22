import os
import sys
import pygame
import logging
import json
from datetime import datetime
from typing import Tuple, Optional, List
from gomoku.game import Game

logger = logging.getLogger(__name__)

WINDOW_HEIGHT = 800
WINDOW_WIDTH = 800

class Color:
    black = (0,0,0)
    white = (200, 200, 200)

class Button:
    LEFT_CLICK = 1
    MIDDLE_CLICK = 2
    RIGHT_CLICK = 3
    SCROLL_UP = 4
    SCROLL_DOWN = 5

class Controller(object):
    offsetY = 62
    offsetX = 64
    pSize = 37.5
    pHeigth = 37.5 / 2


    def __init__(self, screen,  game: Game):
        super()
        self.font = pygame.font.SysFont('Comic Sans MS', 30)
        self.screen = screen
        self.game = game
        self.bg = pygame.image.load(os.path.join(os.path.dirname(__file__), "images", "board.jpg"))
        self.aiPiece = pygame.image.load(os.path.join(os.path.dirname(__file__), "images", "blackPiece.png"))
        self.plPiece = pygame.image.load(os.path.join(os.path.dirname(__file__), "images", "whitePiece.png"))
        self.pointerImg = pygame.image.load(os.path.join(os.path.dirname(__file__), "images", "pointer.png"))


    def draw(self, board: Optional[List[List[int]]] = None, highlight: Optional[List[Tuple[int, int, Tuple[int,int,int]]]] = None):
        """
        board - Draw custom board instead of game.board
        highlight - Highlight pieces at x, y and color of R G B  [(1, 1, (255, 255, 255)), (...), ...]
        """
        highlight = highlight or []
        board = board or self.game.board
        self.screen.fill(Color.black)
        self.screen.blit(self.bg, (0, 0))
        for y, row in enumerate(board):
            for x, v in enumerate(row):
                if v == 2:
                    self.screen.blit(self.aiPiece, self.coordToPos(x, y))
                elif v == 1:
                    self.screen.blit(self.plPiece, self.coordToPos(x, y))
        pointer = self.posToCoord(*pygame.mouse.get_pos())
        if pointer:
            self.screen.blit(self.pointerImg, self.coordToPos(*pointer))
        for x, y, color in highlight:
            pygame.draw.circle(self.screen, color, tuple(map(lambda v: v - self.pHeigth, self.coordToPos(x, y))), self.pHeigth, width=3)
        pygame.display.update()

    def coordToPos(self, x, y) -> Tuple[int, int]:
        return int(x*self.pSize + self.offsetX - self.pHeigth), int(y*self.pSize + self.offsetY -self.pHeigth)

    def posToCoord(self, x, y) -> Optional[Tuple[int, int]]:
        x = x - self.offsetX + self.pHeigth
        y = y - self.offsetY + self.pHeigth
        if x <= 0 or y <= 0:
            return
        cx = int(x / self.pSize)
        cy = int(y / self.pSize)
        if cx > self.game.board_size - 1 or cy > self.game.board_size - 1:
            return
        return cx, cy

    def pieceUnderMouse(self):
        return self.posToCoord(*pygame.mouse.get_pos())

    def onKey(self, key: int):
        if key == pygame.K_s and pygame.key.get_mods() & pygame.KMOD_CTRL:
            json.dump(self.game.board, open(f'board_{datetime.now().strftime("%H-%M-%S")}.json', 'w'))



    def onMouseClick(self, button, x, y):
        logger.debug("onMouseClick: code: %s  %s %s", button, x, y)
        if button == Button.LEFT_CLICK:
            self.onMouseLeftClick(x, y)

    def onMouseLeftClick(self, x, y):
        logger.debug("onMouseLeftClick: %s %s", x, y)
        self.makeMove()

    def makeMove(self):
        p = self.pieceUnderMouse()
        if not p:
            logger.debug("Mouse not over valid position!")
            return False
        x, y = p
        if not self.game.is_valid_move(x, y, 1):
            logger.debug("Position at %s %s occupied or move not valid! Can't make move!", x, y)
            return False

        self.game.set(x, y, 1)
        self.game.next_move()
        return True

    def waitKey(self, key: str, message=None):
        # pylint: disable=no-member
        logger.info("Waiting key: %s message: %s", key, message)
        pygame.display.set_caption(f"Gomoku (waiting for key press: '{key}')")
        while True:
            for event in pygame.event.get():
                if event.type == pygame.KEYUP and pygame.key.name(event.key) == key:
                    logger.debug("Key %s arrived!", key)
                    pygame.display.set_caption("Gomoku")
                    return

                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()



def main():
    # pylint: disable=no-member
    pygame.init()
    pygame.font.init()
    screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
    pygame.display.set_caption("Gomoku")
    screen.fill(Color.black)
    game = Game()
    try:
        if len(sys.argv) > 1:
            game.board = json.load(open(sys.argv[1], 'r'))
    except:
        pass
    controller = Controller(screen, game)
    game.controller = controller
    while True:
        try:
            controller.draw()
        except Exception:
            logger.exception("Draw failed")
        for event in pygame.event.get():
            logger.debug("pygame: event: %s", pygame.event.event_name(event.type))
            if event.type == pygame.QUIT:
                pygame.quiAt()
                return
            try:
                if event.type == pygame.MOUSEBUTTONUP:
                    controller.onMouseClick(event.button, *pygame.mouse.get_pos())
                elif event.type == pygame.KEYUP:
                    controller.onKey(event.key)
            except Exception:
                logger.exception("Failed to process event")


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG if os.environ.get("DEBUG") else logging.INFO)
    logger = logging.getLogger(__name__)
    main()

