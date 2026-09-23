# 🏆 Plateformer — Niveau 2

Jeu de plateforme 2D développé en **C** avec **SDL2**, sous **Linux**.

![Langage](https://img.shields.io/badge/langage-C-blue)
![Bibliothèque](https://img.shields.io/badge/lib-SDL2-green)
![Plateforme](https://img.shields.io/badge/OS-Linux-orange)

## 📖 Description

Un jeu de plateforme classique où le joueur doit se déplacer à travers une rue parisienne stylisée, éviter les ennemis mobiles, collecter des pièces d'or et gérer sa vie tout en composant avec des plateformes fixes et mobiles ainsi que des zones de ralentissement.

## 🎮 Commandes

| Touche | Action |
|--------|--------|
| ← → | Déplacement |
| ↑ | Sauter |
| P | Pause |
| G | Afficher le guide |

## 🎯 Objectifs

- Collecter les **11 pièces d'or** (chaque pièce rapporte 10 points)
- Gérer les **3 cœurs de vie** : une vie est perdue à chaque contact avec un ennemi
- Traverser ou éviter les **zones bleues translucides** (ralentissement temporaire)
- Utiliser les **plateformes bleues mobiles** pour progresser dans le niveau
- Terminer le niveau le plus rapidement possible (chronomètre affiché à l'écran)

## 🧩 Éléments du jeu

| Élément | Description |
|---------|--------------|
| 🟥 Carré rouge | Ennemi mobile |
| 🟩 Carré vert | Le joueur |
| 🟫 Barre marron | Plateforme fixe |
| 🟦 Barre bleue | Plateforme mobile |
| 🟨 Carré jaune | Pièce à collecter (10 pts) |
| 🔵 Zone bleue translucide | Zone de ralentissement temporaire |
| ❤️ Cœurs (haut gauche) | Vies restantes du joueur |

## 🖥️ Interface

- **Score** : affiché en haut à droite (pièces collectées × 10 pts)
- **Niveau** : indicateur du niveau en cours
- **Temps** : chronomètre de progression
- **Vies** : cœurs affichés en haut à gauche

## 🛠️ Technologies

- **Langage** : C
- **Bibliothèque graphique** : SDL2
- **Plateforme cible** : Linux

## 📦 Prérequis

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

## ⚙️ Compilation

```bash
gcc main.c -o game $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf)
```

## ▶️ Exécution

```bash
./game
```
.
