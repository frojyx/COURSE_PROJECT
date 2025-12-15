// TemplateUtils.h
#ifndef TEMPLATEUTILS_H
#define TEMPLATEUTILS_H

#include <QList>
#include <QString>
#include <algorithm>
#include <functional>

// Шаблонная функция для поиска элемента в контейнере по предикату
template<typename Container, typename Predicate>
auto findIf(const Container& container, Predicate pred) -> decltype(container.begin()) {
    return std::find_if(container.begin(), container.end(), pred);
}

// Шаблонная функция для фильтрации контейнера по предикату
template<typename Container, typename Predicate>
QList<typename Container::value_type> filter(const Container& container, Predicate pred) {
    QList<typename Container::value_type> result;
    std::copy_if(container.begin(), container.end(), std::back_inserter(result), pred);
    return result;
}

// Шаблонная функция для проверки, содержит ли контейнер элемент, удовлетворяющий предикату
template<typename Container, typename Predicate>
bool containsIf(const Container& container, Predicate pred) {
    return std::any_of(container.begin(), container.end(), pred);
}

// Шаблонная функция для подсчета элементов, удовлетворяющих предикату
template<typename Container, typename Predicate>
int countIf(const Container& container, Predicate pred) {
    return std::count_if(container.begin(), container.end(), pred);
}

// Шаблонная функция для преобразования элементов контейнера
template<typename Container, typename Transform>
QList<decltype(std::declval<Transform>()(std::declval<typename Container::value_type>()))>
map(const Container& container, Transform transform) {
    QList<decltype(std::declval<Transform>()(std::declval<typename Container::value_type>()))> result;
    std::transform(container.begin(), container.end(), std::back_inserter(result), transform);
    return result;
}

// Шаблонная функция для сортировки контейнера по компаратору
template<typename Container, typename Compare>
void sort(Container& container, Compare comp) {
    std::sort(container.begin(), container.end(), comp);
}

// Шаблонный класс для сравнения по полю объекта
template<typename T, typename MemberType>
class MemberComparator {
public:
    using MemberPtr = MemberType (T::*)() const;
    
    MemberComparator(MemberPtr member, bool ascending = true)
        : memberPtr(member), ascending(ascending) {}
    
    bool operator()(const T& a, const T& b) const {
        MemberType valA = (a.*memberPtr)();
        MemberType valB = (b.*memberPtr)();
        return ascending ? (valA < valB) : (valA > valB);
    }
    
private:
    MemberPtr memberPtr;
    bool ascending;
};

// Вспомогательная функция для создания компаратора по полю
template<typename T, typename MemberType>
MemberComparator<T, MemberType> makeMemberComparator(
    MemberType (T::*member)() const, bool ascending = true) {
    return MemberComparator<T, MemberType>(member, ascending);
}

// Шаблонный класс для функционального компаратора
template<typename T, typename CompareFunc>
class FunctionalComparator {
public:
    FunctionalComparator(CompareFunc func, bool ascending = true)
        : compareFunc(func), ascending(ascending) {}
    
    bool operator()(const T& a, const T& b) const {
        auto valA = compareFunc(a);
        auto valB = compareFunc(b);
        return ascending ? (valA < valB) : (valA > valB);
    }
    
private:
    CompareFunc compareFunc;
    bool ascending;
};

// Вспомогательная функция для создания функционального компаратора
template<typename T, typename CompareFunc>
FunctionalComparator<T, CompareFunc> makeFunctionalComparator(
    CompareFunc func, bool ascending = true) {
    return FunctionalComparator<T, CompareFunc>(func, ascending);
}

// Шаблонная функция для получения максимального элемента по компаратору
template<typename Container, typename Compare>
typename Container::const_iterator maxElement(const Container& container, Compare comp) {
    return std::max_element(container.begin(), container.end(), comp);
}

// Шаблонная функция для получения минимального элемента по компаратору
template<typename Container, typename Compare>
typename Container::const_iterator minElement(const Container& container, Compare comp) {
    return std::min_element(container.begin(), container.end(), comp);
}

// Шаблонная функция для проверки, отсортирован ли контейнер
template<typename Container, typename Compare>
bool isSorted(const Container& container, Compare comp) {
    return std::is_sorted(container.begin(), container.end(), comp);
}

// Шаблонная функция для удаления дубликатов из отсортированного контейнера
template<typename Container>
void removeDuplicates(Container& container) {
    container.erase(std::unique(container.begin(), container.end()), container.end());
}

// Шаблонная функция для удаления дубликатов с использованием компаратора
template<typename Container, typename Compare>
void removeDuplicates(Container& container, Compare comp) {
    container.erase(std::unique(container.begin(), container.end(), comp), container.end());
}

// Шаблонный класс для обертки контейнера с дополнительными методами
template<typename T>
class ContainerWrapper {
public:
    using ContainerType = QList<T>;
    using Iterator = typename ContainerType::iterator;
    using ConstIterator = typename ContainerType::const_iterator;
    
    ContainerWrapper() = default;
    ContainerWrapper(const ContainerType& container) : data(container) {}
    ContainerWrapper(ContainerType&& container) : data(std::move(container)) {}
    
    // Итераторы
    Iterator begin() { return data.begin(); }
    Iterator end() { return data.end(); }
    ConstIterator begin() const { return data.begin(); }
    ConstIterator end() const { return data.end(); }
    ConstIterator cbegin() const { return data.begin(); }
    ConstIterator cend() const { return data.end(); }
    
    // Доступ к данным
    ContainerType& get() { return data; }
    const ContainerType& get() const { return data; }
    
    // Размер
    int size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    
    // Добавление элементов
    void append(const T& value) { data.append(value); }
    void append(T&& value) { data.append(std::move(value)); }
    void prepend(const T& value) { data.prepend(value); }
    
    // Удаление элементов
    void removeAt(int index) { data.removeAt(index); }
    bool removeOne(const T& value) { return data.removeOne(value); }
    void clear() { data.clear(); }
    
    // Поиск
    template<typename Predicate>
    Iterator findIf(Predicate pred) {
        return std::find_if(data.begin(), data.end(), pred);
    }
    
    template<typename Predicate>
    ConstIterator findIf(Predicate pred) const {
        return std::find_if(data.begin(), data.end(), pred);
    }
    
    // Фильтрация
    template<typename Predicate>
    ContainerWrapper filter(Predicate pred) const {
        ContainerWrapper result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result.data), pred);
        return result;
    }
    
    // Преобразование
    template<typename Transform>
    auto map(Transform transform) const -> ContainerWrapper<decltype(transform(std::declval<T>()))> {
        ContainerWrapper<decltype(transform(std::declval<T>()))> result;
        std::transform(data.begin(), data.end(), std::back_inserter(result.data), transform);
        return result;
    }
    
    // Сортировка
    template<typename Compare>
    void sort(Compare comp) {
        std::sort(data.begin(), data.end(), comp);
    }
    
private:
    ContainerType data;
};

#endif // TEMPLATEUTILS_H

