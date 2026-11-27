#include "genre.h"

#include <iostream>

void initializeGenreList(GenreList& list) { list.first = nullptr; }

void destroyGenreList(GenreList& list) {
  GenreAddress temp;

  while (!isGenreListEmpty(list)) {
    removeFirstGenre(list, temp);
    delete temp;
  }

  list.first = nullptr;
}

void dumpGenreList(GenreList list) {
  GenreAddress current;

  current = list.first;
  while (current != nullptr) {
    std::cout << current->info.name;
    if (current->next != nullptr) {
      std::cout << ", ";
    }
    current = current->next;
  }
  std::cout << std::endl;
}

bool isGenreListEmpty(GenreList list) { return list.first == nullptr; }

GenreAddress searchGenre(GenreList list, std::string name) {
  GenreAddress current;

  current = list.first;
  while (current != nullptr && current->info.name != name) {
    current = current->next;
  }

  return current;
}

bool doesGenreExist(GenreList list, std::string name) {
  return searchGenre(list, name) != nullptr;
}

GenreAddress createGenreElement(std::string name) {
  GenreAddress new_genre = new GenreElement;
  new_genre->info.name = name;
  new_genre->next = nullptr;
  return new_genre;
}

void addGenre(GenreList& list, std::string name) {
  GenreAddress new_genre;

  if (!doesGenreExist(list, name)) {
    new_genre = createGenreElement(name);
    insertGenreLast(list, new_genre);
  }
}

void removeGenre(GenreList& list, std::string name) {
  GenreAddress prev, current, temp;

  prev = nullptr;
  current = list.first;

  while (current != nullptr && current->info.name != name) {
    prev = current;
    current = current->next;
  }

  if (current == list.first) {
    removeFirstGenre(list, temp);
  } else if (current->next == nullptr) {
    removeLastGenre(list, temp);
  } else {
    removeAfterGenre(list, prev, temp);
  }

  delete temp;
}

void insertGenreFirst(GenreList& list, GenreAddress new_genre) {
  GenreAddress current;

  if (isGenreListEmpty(list)) {
    list.first = new_genre;
  } else {
    new_genre->next = list.first;
    list.first = new_genre;
  }
}

void insertGenreLast(GenreList& list, GenreAddress new_genre) {
  GenreAddress current;

  if (isGenreListEmpty(list)) {
    list.first = new_genre;
  } else {
    current = list.first;
    while (current->next != nullptr) {
      current = current->next;
    }
    current->next = new_genre;
  }
}

void insertGenreAfter(GenreList& list, GenreAddress target, GenreAddress new_genre) {
  GenreAddress current;

  current = list.first;
  while (current != nullptr && current != target) {
    current = current->next;
  }

  if (current != nullptr) {
    if (current->next == nullptr) {
      insertGenreLast(list, new_genre);
    } else {
      new_genre->next = current->next;
      current->next = new_genre;
    }
  }
}

void removeFirstGenre(GenreList& list, GenreAddress& out) {
  out = nullptr;

  if (!isGenreListEmpty(list)) {
    GenreAddress genre = list.first;
    list.first = list.first->next;
    genre->next = nullptr;
    out = genre;
  }
}

void removeLastGenre(GenreList& list, GenreAddress& out) {
  out = nullptr;

  if (!isGenreListEmpty(list)) {
    GenreAddress prev = nullptr;
    GenreAddress current = list.first;

    while (current->next != nullptr) {
      prev = current;
      current = current->next;
    }

    if (prev == nullptr) {
      removeFirstGenre(list, out);
    } else {
      GenreAddress genre = current;
      prev->next = current->next;
      genre->next = nullptr;
      out = genre;
    }
  }
}

void removeAfterGenre(GenreList& list, GenreAddress target, GenreAddress& out) {
  out = nullptr;

  if (!isGenreListEmpty(list) && target != nullptr) {
    GenreAddress current = list.first;
    while (current != nullptr && current != target) {
      current = current->next;
    }

    if (current != nullptr && current->next != nullptr) {
      GenreAddress genre = current->next;
      current->next = current->next->next;
      genre->next = nullptr;
      out = genre;
    }
  }
}