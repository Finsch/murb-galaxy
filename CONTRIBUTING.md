
## Guide de contribution - Projet MUrB v3

### 📋 Workflow Git

#### Types de branches

- **`main`** : Code stable et validé (version finale)
- **`develop`** : Code en cours d'intégration
- **`feature/*`** : Nouvelles fonctionnalités

#### Convention de nommage des branches

```
feature/seq           # Optimisation séquentielle
feature/simd          # Version SIMD
feature/threads       # Parallélisation OpenMP
feature/gpu           # Version GPU
feature/hybrid        # Version hétérogène
```

#### Messages de commit

Utiliser le format conventionnel :

- feat: Nouvelle fonctionnalité
- fix: Correction de bug
- docs: Documentation
- test: Tests

---

### 🚀 Guide pas à pas.

Situation : Bob travaille sur l'optimisation séquentielle.
#### Étape 1 : Bob prépare son environnement

```bash
# 1. Clone le dépôt
git clone https://github.com/Finsch/murb-galaxy.git
cd murb-galaxy

# 2. Récupère la dernière version
git checkout develop
git pull origin develop

# 3. Crée SA branche de feature
git checkout -b feature/seq  

# 4. Pousse la branche sur GitHub
git push -u origin feature/seq
```

#### Étape 2 : Bob développe localement

```bash
# Travaille sur ton implémentation
# Modifie src/murb/implem/SimulationNBodyOptim.{hpp,cpp}
# Modifie main.cpp pour ajouter le tag cpu+optim
# Tester et valider.

# Commite régulièrement
git add src/murb/implem/SimulationNBodyOptim.cpp
git commit -m "feat: implémentation optimisation séquentielle"

# Pousse tes changements
git push origin feature/seq
```

#### Étape 3 : Bob crée une Pull Request (PR)

1. Va sur https://github.com/Finsch/murb-galaxy
2. Clique sur "Pull requests" → "New pull request"
3. Configure :
   - **base:** `develop` ← Où on veut merger
   - **compare:** `feature/sequential` ← Ce qu'on veut merger
4. Remplis le template :
```
Titre: [FEAT] Implémentation de l'optimisation séquentielle

Description:

## Validation
Tous les tests passent
Résultats identiques au golden model

## Performance
Avant (naive): 15 FPS
Après (optim): 22 FPS (+46%)

## Commandes de test
./bin/murb -n 5000 -i 1000 --nv --im cpu+naive
./bin/murb -n 5000 -i 1000 --nv --im cpu+optim
```
5. Assign 2 reviewers (les autres membres de l'équipe)
6. Clique "Create pull request"

---

- Alice et Charlie reçoivent une notification
- Ils examinent le code :
- Vérifient la qualité
- Testent localement si besoin
- Donnent leur approbation ("Approve") ou commentaires

#### Étape 4 : Après approbation, merge de la feature.

- Option 1 : Merge via GitHub (recommandé)
	- Clique "Merge pull request" 
	- Choisir **"Squash and merge"** pour un historique propre

- Option 2 : Merge manuel
```bash
git checkout develop
git pull origin develop
git merge --squash feature/seq
git commit -m "feat: implémentation de l'optimisation séquentielle"
git push origin develop
```

#### Étape 5 : Nettoyage

```
# Supprime la branche locale
git branch -d feature/seq

# Supprime la branche sur GitHub
git push origin --delete feature/seq
```

---

##### petit resumé.

| Étape | Qui        | Action                              | Résultat                  |
| ----- | ---------- | ----------------------------------- | ------------------------- |
| 1     | **Ugo**    | Crée repo avec `main` et `develop`  | Base de travail           |
| 2     | **Bob**    | Clone, crée `feature/sequential`    | Travaille localement      |
| 3     | **Bob**    | Développe, teste, push              | Code sauvegardé           |
| 4     | **Bob**    | PR vers `develop`                   | Code intégré après review |
| 5     | **Équipe** | Répète 2-4 pour toutes les features | `develop` contient tout   |
| 6     | **Équipe** | Merge `develop` → `main`            | Version finale livrée     |

---

### 🔄 Dernière étape : Merge `develop` → `main`

Quand TOUTES les features sont dans `develop` et que tout est validé :

#### Option 1 : Via GitHub 

1. Aller sur GitHub → Pull requests → New
2. Configurer :
   - **base:** `main`
   - **compare:** `develop`
3. Titre : "Release: Version finale projet MUrB"
4. Description : Résumé de toutes les implémentations
5. Créer la PR
6. **Assign tous les membres** comme reviewers
7. Après approbation, **"Merge pull request"**

#### Option 2 : En ligne de commande //ici

```bash
# 1. Synchronise tout
git checkout main
git pull origin main
git checkout develop
git pull origin develop

# 2. Merge develop dans main
git checkout main
git merge --no-ff develop -m "release: version finale projet MUrB"

# 3. Pousse sur GitHub
git push origin main
```

---

#### 📋 Checklist finale avant merge develop→main

- [ ] Toutes les implémentations sont dans `develop`
- [ ] Tous les tests passent (`./bin/murb-test`)
- [ ] Benchmarks documentés
- [ ] Rapport rédigé
- [ ] Code commenté
- [ ] README à jour
- [ ] Aucun bug connu

---


---
### ⚡ Commandes rapides //ici

push feature.
```bash
# Synchronisation
git checkout develop && git pull origin develop

# Nouvelle feature
git checkout feature/seq

# Commit et push
git add . && git commit -m "feat: description"
git push origin feature/seq
```

push dev.
```bash
git checkout develop
git add .
git status
git commit -m "docs: mis à jour du CONTRIBUTING.md"
git push origin develop
```

main <- dev.
```bash
# 1. Synchronise tout
git checkout main
git pull origin main
git checkout develop
git pull origin develop

# 2. Merge develop dans main
git checkout main
git merge --no-ff develop -m "release: version finale projet MUrB"

# 3. Pousse sur GitHub
git push origin main
```