-- Author: Daniel Klimaj; xklima22@stud.fit.vutbr.cz

module Lib.Utils.Utils where

import Data.Char

-- Checks if string is empty
emptyStr :: String -> Bool
emptyStr [] = True
emptyStr _  = False

-- Splits string into array of strings, each element is string of length 1
splitStr :: String -> [String]
splitStr []    = []
splitStr (h:r) = [h]:(splitStr r)

-- Checks if given char is symbol
isSymbol:: Char -> Bool
isSymbol c
  | (isAlpha c) == True || c == '#' = True
  | otherwise                       = False

-- Checks if given char is terminal
isTerm:: Char -> Bool
isTerm c
  | c == '#'  = True
  | isLower c = True
  | otherwise = False

-- Checks if given char is nonterminal
isNterm :: Char -> Bool
isNterm c
  | isUpper c = True
  | otherwise = False

-- Converts first element of the string to char
s2ch :: String -> Char
s2ch []     = ' '
s2ch (s:rs) = s
