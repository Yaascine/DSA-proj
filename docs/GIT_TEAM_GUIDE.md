# Team Git Guide (simple, for beginners)

This is the shared workflow for the 4-person team. Follow it so nobody overwrites
anyone else's work and the frozen contracts stay safe.

## Golden rules
1. `main` always works. **Never write code directly on `main`.**
2. One task = one **branch**.
3. Changes go through a **Pull Request (PR)**; tests must pass (CI checks this).
4. **One person merges** (P1). This protects the shared headers.
5. Pull the latest `main` before starting new work.

## One-time setup (P1 does this)
1. Create a GitHub repository (e.g. `TheGodExperiment`).
2. Push this project to it:
   ```
   git remote add origin https://github.com/<your-name>/TheGodExperiment.git
   git branch -M main
   git push -u origin main
   ```
3. Add teammates: repo **Settings → Collaborators → Add people** (their GitHub usernames).
4. Turn on branch protection (recommended): **Settings → Branches → Add rule** for `main`
   → require pull request + require the CI check to pass.

## Every teammate, every task
```
# 1. Get the latest main
git checkout main
git pull

# 2. Make a branch for your task (name it after your part + task)
git checkout -b p2-pathfinding

# 3. ...do the work, run the tests...
make test

# 4. Save your work
git add <the files you changed>
git commit -m "P2: implement Dijkstra findPath"

# 5. Push your branch
git push -u origin p2-pathfinding
```
Then on GitHub: open a **Pull Request** from your branch into `main`. Describe what you
did and paste your real test output. P1 reviews and merges.

## After your PR is merged
```
git checkout main
git pull
git branch -d p2-pathfinding   # tidy up the finished branch
```

## If you get a conflict
Don't panic and don't delete things. Git marks the conflicting lines; edit them to keep
the correct version, then `git add <file>` and `git commit`. Ask P1 if unsure.

## Contract changes
Shared headers (`Types.h`, `World.h`, `Human.h`, `History.h`, `Command.h`, `Snapshot.h`,
`ISystem.h`) are **frozen**. To change one, agree with the WHOLE team first, then make a
PR titled "CONTRACT CHANGE: ...". Never slip a contract change into a normal PR.
