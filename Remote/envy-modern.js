/**
 * Envy Remote Control Interface - Modern JavaScript
 * Version 2026 - Enhanced UX with AJAX and Real-time Updates
 */

(function() {
    'use strict';

    // Configuration
    const CONFIG = {
        refreshInterval: 30000, // 30 seconds
        ajaxTimeout: 10000,    // 10 seconds
        retryAttempts: 3,
        enableNotifications: true
    };

    // Application State
    const state = {
        currentPage: null,
        isLoggedIn: false,
        refreshTimer: null,
        csrfToken: null,
        lastUpdate: null
    };

    // DOM Cache
    const cache = {};

    /**
     * Initialize the application
     */
    function init() {
        cacheElements();
        setupEventListeners();
        initializeFeatures();
        hideLoadingScreen();
        startPeriodicRefresh();
    }

    /**
     * Cache frequently used DOM elements
     */
    function cacheElements() {
        cache.loadingScreen = document.getElementById('loading-screen');
        cache.mainContent = document.getElementById('main-content');
        cache.navTabs = document.querySelector('.nav-tabs');
    }

    /**
     * Setup event listeners
     */
    function setupEventListeners() {
        // Navigation
        if (cache.navTabs) {
            cache.navTabs.addEventListener('click', handleNavigation);
        }

        // Form submissions
        document.addEventListener('submit', handleFormSubmit);

        // Keyboard shortcuts
        document.addEventListener('keydown', handleKeyboardShortcuts);

        // Online/Offline detection
        window.addEventListener('online', handleOnlineStatus);
        window.addEventListener('offline', handleOfflineStatus);

        // Page visibility
        document.addEventListener('visibilitychange', handleVisibilityChange);
    }

    /**
     * Initialize additional features
     */
    function initializeFeatures() {
        setupCSRFProtection();
        initializeNotifications();
        setupResponsiveFeatures();
        initializeTooltips();
    }

    /**
     * Setup CSRF protection
     */
    function setupCSRFProtection() {
        // Generate CSRF token
        state.csrfToken = window.EnvySecurity && typeof window.EnvySecurity.getCSRFToken === 'function'
            ? window.EnvySecurity.getCSRFToken()
            : generateCSRFToken();

        // Add to all forms
        document.querySelectorAll('form').forEach(form => {
            const csrfInput = document.createElement('input');
            csrfInput.type = 'hidden';
            csrfInput.name = 'csrf_token';
            csrfInput.value = state.csrfToken;
            form.appendChild(csrfInput);
        });
    }

    /**
     * Handle navigation clicks
     */
    function handleNavigation(e) {
        if (e.target.tagName === 'A') {
            e.preventDefault();
            const url = e.target.getAttribute('href');
            navigateTo(url);
        }
    }

    /**
     * Navigate to a new page with AJAX
     */
    function navigateTo(url, pushState = true) {
        const safeUrl = window.EnvySecurityUtils.validateRedirectTarget(url, ['/remote', '/api']);
        if (!safeUrl) {
            console.warn('Blocked unsafe navigation target:', url);
            return;
        }

        showLoadingIndicator();

        ajaxRequest(safeUrl, {
            method: 'GET',
            headers: {
                'X-Requested-With': 'XMLHttpRequest',
                'X-CSRF-Token': state.csrfToken
            }
        })
        .then(response => {
            updateContent(response.html);
            if (pushState) {
                history.pushState({url: safeUrl}, '', safeUrl);
            }
            updateNavigationState(safeUrl);
            hideLoadingIndicator();
        })
        .catch(error => {
            console.error('Navigation failed:', error);
            // Fallback to regular navigation
            const safeFallback = window.EnvySecurityUtils.validateRedirectTarget(safeUrl, ['/remote']);
            window.location.href = safeFallback || '/remote/home';
        });
    }

    /**
     * Handle form submissions
     */
    function handleFormSubmit(e) {
        const form = e.target;

        // Check if form should use AJAX
        if (form.hasAttribute('data-ajax')) {
            e.preventDefault();
            submitFormAjax(form);
        }
    }

    /**
     * Submit form via AJAX
     */
    function submitFormAjax(form) {
        const formData = new FormData(form);
        const rawAction = form.getAttribute('action');
        // Resolve relative and empty actions against current location before validating
        let resolvedAction;
        try {
            resolvedAction = rawAction
                ? new URL(rawAction, window.location.href).pathname
                : window.location.pathname;
        } catch (_) {
            resolvedAction = window.location.pathname;
        }
        const safeAction = window.EnvySecurityUtils.validateRedirectTarget(resolvedAction, ['/remote', '/api']);
        if (!safeAction) {
            showNotification('Invalid form action.', 'error');
            return;
        }

        showLoadingIndicator();

        let method;
        try {
            method = window.EnvySecurityUtils.validateHttpMethod(form.method || 'POST');
        } catch (error) {
            showNotification('Unsupported form method.', 'error');
            return;
        }

        ajaxRequest(safeAction, {
            method,
            body: formData,
            headers: {
                'X-Requested-With': 'XMLHttpRequest',
                'X-CSRF-Token': state.csrfToken
            }
        })
        .then(response => {
            if (response.success) {
                showNotification(response.message || 'Operation completed successfully', 'success');
                if (response.redirect) {
                    const safeRedirect = window.EnvySecurityUtils.validateRedirectTarget(response.redirect, ['/remote']);
                    if (safeRedirect) {
                        setTimeout(() => navigateTo(safeRedirect), 1000);
                    }
                } else if (response.refresh) {
                    refreshCurrentPage();
                }
            } else {
                showNotification(response.message || 'Operation failed', 'error');
            }
            hideLoadingIndicator();
        })
        .catch(error => {
            console.error('Form submission failed:', error);
            showNotification('Request failed. Please try again.', 'error');
            hideLoadingIndicator();
        });
    }

    /**
     * AJAX request utility
     */
    function ajaxRequest(url, options = {}) {
        const defaultOptions = {
            method: 'GET',
            headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json'
            },
            timeout: CONFIG.ajaxTimeout
        };

        const finalOptions = { ...defaultOptions, ...options };

        // Handle FormData
        if (finalOptions.body instanceof FormData) {
            delete finalOptions.headers['Content-Type'];
        } else if (typeof finalOptions.body === 'object') {
            finalOptions.body = JSON.stringify(finalOptions.body);
        }

        return new Promise((resolve, reject) => {
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), finalOptions.timeout);

            fetch(url, {
                ...finalOptions,
                signal: controller.signal
            })
            .then(response => {
                clearTimeout(timeoutId);
                if (!response.ok) {
                    throw new Error(`HTTP ${response.status}: ${response.statusText}`);
                }
                return response.json();
            })
            .then(resolve)
            .catch(error => {
                clearTimeout(timeoutId);
                reject(error);
            });
        });
    }

    /**
     * Update main content area
     */
    function updateContent(html) {
        if (cache.mainContent) {
            cache.mainContent.innerHTML = window.EnvySecurityUtils.sanitizeHTML(html);
            initializeDynamicContent();
        }
    }

    /**
     * Initialize dynamic content after AJAX updates
     */
    function initializeDynamicContent() {
        // Re-initialize tooltips
        initializeTooltips();

        // Setup progress bars
        initializeProgressBars();

        // Setup sortable tables
        initializeSortableTables();
    }

    /**
     * Show loading indicator
     */
    function showLoadingIndicator() {
        const indicator = document.querySelector('.loading-indicator') ||
                         createLoadingIndicator();
        indicator.style.display = 'block';
    }

    /**
     * Hide loading indicator
     */
    function hideLoadingIndicator() {
        const indicator = document.querySelector('.loading-indicator');
        if (indicator) {
            indicator.style.display = 'none';
        }
    }

    /**
     * Create loading indicator
     */
    function createLoadingIndicator() {
        const indicator = document.createElement('div');
        indicator.className = 'loading-indicator';

        const content = document.createElement('div');
        content.className = 'loading-content';

        const spinner = document.createElement('div');
        spinner.className = 'loading-spinner';

        const label = document.createElement('span');
        label.textContent = 'Loading...';

        content.appendChild(spinner);
        content.appendChild(label);
        indicator.appendChild(content);

        document.body.appendChild(indicator);
        return indicator;
    }

    /**
     * Show notification
     */
    function showNotification(message, type = 'info') {
        const notification = createNotification(message, type);
        document.body.appendChild(notification);

        // Auto-remove after 5 seconds
        setTimeout(() => {
            notification.remove();
        }, 5000);
    }

    /**
     * Create notification element
     */
    function createNotification(message, type) {
        const notification = document.createElement('div');
        const safeType = ['info', 'success', 'warning', 'error'].includes(type) ? type : 'info';
        notification.className = `alert alert-${safeType} envy-toast`;

        const text = document.createElement('span');
        text.textContent = String(message || '');

        const close = document.createElement('button');
        close.type = 'button';
        close.setAttribute('aria-label', 'Close notification');
        close.textContent = '×';
        close.addEventListener('click', () => notification.remove());

        notification.appendChild(text);
        notification.appendChild(close);

        return notification;
    }

    /**
     * Initialize notifications system
     */
    function initializeNotifications() {
        if ('Notification' in window && CONFIG.enableNotifications) {
            if (Notification.permission === 'default') {
                Notification.requestPermission();
            }
        }
    }

    /**
     * Handle keyboard shortcuts
     */
    function handleKeyboardShortcuts(e) {
        // Ctrl/Cmd + R for refresh
        if ((e.ctrlKey || e.metaKey) && e.key === 'r') {
            e.preventDefault();
            refreshCurrentPage();
        }

        // Escape to close modals
        if (e.key === 'Escape') {
            closeModals();
        }
    }

    /**
     * Refresh current page
     */
    function refreshCurrentPage() {
        navigateTo(window.location.href, false);
    }

    /**
     * Start periodic refresh
     */
    function startPeriodicRefresh() {
        state.refreshTimer = setInterval(() => {
            if (!document.hidden) {
                performSilentRefresh();
            }
        }, CONFIG.refreshInterval);
    }

    /**
     * Perform silent refresh (update data without full page reload)
     */
    function performSilentRefresh() {
        // This would update specific elements like download progress
        // without full page refresh
        updateDynamicContent();
    }

    /**
     * Update dynamic content
     */
    function updateDynamicContent() {
        // Update progress bars
        updateProgressBars();

        // Update statistics
        updateStatistics();

        // Update timestamps
        updateTimestamps();
    }

    /**
     * Initialize progress bars
     */
    function initializeProgressBars() {
        document.querySelectorAll('.progress-bar').forEach(bar => {
            const percentage = bar.getAttribute('data-percentage') || 0;
            bar.style.width = percentage + '%';
        });
    }

    /**
     * Update progress bars
     */
    function updateProgressBars() {
        document.querySelectorAll('[data-progress-id]').forEach(element => {
            const id = element.getAttribute('data-progress-id');
            // Fetch updated progress from server
            fetchProgressUpdate(id);
        });
    }

    /**
     * Fetch progress update
     */
    function fetchProgressUpdate(id) {
        try {
            window.EnvySecurityUtils.validateApiSegment(id, 'progress id');
        } catch (error) {
            if (error.name === 'ValidationError') {
                console.warn('Invalid progress id:', error.code);
                return;
            }
            throw error;
        }

        ajaxRequest(`/api/progress/${id}`)
            .then(data => {
                updateProgressElement(id, data);
            })
            .catch(error => {
                console.warn('Failed to update progress:', error);
            });
    }

    /**
     * Update progress element
     */
    function updateProgressElement(id, data) {
        const element = document.querySelector(`[data-progress-id="${id}"]`);
        if (element) {
            element.style.width = data.percentage + '%';
            element.setAttribute('aria-valuenow', data.percentage);

            // Update status text
            const statusElement = element.closest('.progress-container')
                                   .querySelector('.progress-text');
            if (statusElement) {
                statusElement.textContent = `${data.percentage}% - ${data.status}`;
            }
        }
    }

    /**
     * Initialize sortable tables
     */
    function initializeSortableTables() {
        document.querySelectorAll('.table-sortable').forEach(table => {
            const headers = table.querySelectorAll('th[data-sort]');
            headers.forEach(header => {
                header.style.cursor = 'pointer';
                header.addEventListener('click', () => sortTable(table, header));
            });
        });
    }

    /**
     * Sort table
     */
    function sortTable(table, header) {
        const column = header.getAttribute('data-sort');
        const direction = header.getAttribute('data-direction') || 'asc';

        // Update direction
        header.setAttribute('data-direction', direction === 'asc' ? 'desc' : 'asc');

        // Sort rows
        const tbody = table.querySelector('tbody');
        const rows = Array.from(tbody.querySelectorAll('tr'));

        rows.sort((a, b) => {
            const aVal = a.querySelector(`[data-sort-value="${column}"]`)?.textContent || '';
            const bVal = b.querySelector(`[data-sort-value="${column}"]`)?.textContent || '';

            if (direction === 'asc') {
                return aVal.localeCompare(bVal);
            } else {
                return bVal.localeCompare(aVal);
            }
        });

        // Re-append sorted rows
        rows.forEach(row => tbody.appendChild(row));
    }

    /**
     * Initialize tooltips
     */
    function initializeTooltips() {
        document.querySelectorAll('[data-tooltip]').forEach(element => {
            element.addEventListener('mouseenter', showTooltip);
            element.addEventListener('mouseleave', hideTooltip);
        });
    }

    /**
     * Show tooltip
     */
    function showTooltip(e) {
        const tooltip = createTooltip(e.target.getAttribute('data-tooltip'));
        document.body.appendChild(tooltip);

        const rect = e.target.getBoundingClientRect();
        tooltip.style.left = rect.left + (rect.width / 2) + 'px';
        tooltip.style.top = rect.top - 30 + 'px';
    }

    /**
     * Hide tooltip
     */
    function hideTooltip() {
        const tooltip = document.querySelector('.tooltip');
        if (tooltip) {
            tooltip.remove();
        }
    }

    /**
     * Create tooltip element
     */
    function createTooltip(text) {
        const tooltip = document.createElement('div');
        tooltip.className = 'tooltip';
        tooltip.textContent = text;
        tooltip.style.cssText = `
            position: absolute;
            background: rgba(0,0,0,0.8);
            color: white;
            padding: 0.5rem;
            border-radius: 4px;
            font-size: 0.8rem;
            pointer-events: none;
            z-index: 10000;
            transform: translateX(-50%);
        `;
        return tooltip;
    }

    /**
     * Setup responsive features
     */
    function setupResponsiveFeatures() {
        // Mobile menu toggle
        const navToggle = document.createElement('button');
        navToggle.className = 'nav-toggle';
        navToggle.type = 'button';
        navToggle.textContent = '☰';

        const header = document.querySelector('.header');
        if (header) {
            header.appendChild(navToggle);
            navToggle.addEventListener('click', toggleMobileMenu);
        }

        // Show/hide toggle based on screen size
        function checkScreenSize() {
            if (window.innerWidth <= 768) {
                navToggle.classList.add('nav-toggle-visible');
                cache.navTabs.classList.add('nav-tabs-mobile-hidden');
            } else {
                navToggle.classList.remove('nav-toggle-visible');
                cache.navTabs.classList.remove('nav-tabs-mobile-hidden');
                cache.navTabs.classList.remove('nav-tabs-mobile-open');
            }
        }

        window.addEventListener('resize', checkScreenSize);
        checkScreenSize();
    }

    /**
     * Toggle mobile menu
     */
    function toggleMobileMenu() {
        const nav = cache.navTabs;
        if (nav) {
            nav.classList.toggle('nav-tabs-mobile-open');
        }
    }

    /**
     * Handle online status
     */
    function handleOnlineStatus() {
        showNotification('Connection restored', 'success');
        refreshCurrentPage();
    }

    /**
     * Handle offline status
     */
    function handleOfflineStatus() {
        showNotification('Connection lost. Working offline.', 'error');
    }

    /**
     * Handle page visibility change
     */
    function handleVisibilityChange() {
        if (document.hidden) {
            // Pause updates when page is not visible
            clearInterval(state.refreshTimer);
        } else {
            // Resume updates
            startPeriodicRefresh();
            refreshCurrentPage();
        }
    }

    /**
     * Close modals
     */
    function closeModals() {
        document.querySelectorAll('.modal').forEach(modal => {
            modal.style.display = 'none';
        });
    }

    /**
     * Generate CSRF token
     */
    function generateCSRFToken() {
        return window.EnvySecurityUtils.generateSecureToken(16);
    }

    /**
     * Update navigation state
     */
    function updateNavigationState(url) {
        // Update active tab based on URL
        document.querySelectorAll('.nav-tabs a').forEach(link => {
            link.removeAttribute('aria-current');
            if (link.getAttribute('href') === url) {
                link.setAttribute('aria-current', 'page');
                link.parentElement.classList.add('active');
            } else {
                link.parentElement.classList.remove('active');
            }
        });
    }

    /**
     * Update statistics
     */
    function updateStatistics() {
        // Update download/upload speeds, peer counts, etc.
        document.querySelectorAll('[data-stat]').forEach(element => {
            const stat = element.getAttribute('data-stat');
            fetchStatUpdate(stat);
        });
    }

    /**
     * Fetch stat update
     */
    function fetchStatUpdate(stat) {
        try {
            window.EnvySecurityUtils.validateApiSegment(stat, 'stat');
        } catch (error) {
            if (error.name === 'ValidationError') {
                console.warn('Invalid stat parameter:', error.code);
                return;
            }
            throw error;
        }

        ajaxRequest(`/api/stats/${stat}`)
            .then(data => {
                if (!data || (typeof data.value !== 'string' && typeof data.value !== 'number')) {
                    throw new window.EnvySecurityUtils.ValidationError('INVALID_RESPONSE', 'Unexpected stat response shape');
                }

                const element = document.querySelector(`[data-stat="${stat}"]`);
                if (element) {
                    element.textContent = String(data.value);
                }
            })
            .catch(error => {
                console.warn('Failed to update stat:', error);
            });
    }

    /**
     * Update timestamps
     */
    function updateTimestamps() {
        document.querySelectorAll('[data-timestamp]').forEach(element => {
            const timestamp = parseInt(element.getAttribute('data-timestamp'));
            const now = Date.now();
            const diff = now - timestamp;

            element.textContent = formatTimeAgo(diff);
        });
    }

    /**
     * Format time ago
     */
    function formatTimeAgo(diff) {
        const seconds = Math.floor(diff / 1000);
        const minutes = Math.floor(seconds / 60);
        const hours = Math.floor(minutes / 60);
        const days = Math.floor(hours / 24);

        if (days > 0) return `${days}d ago`;
        if (hours > 0) return `${hours}h ago`;
        if (minutes > 0) return `${minutes}m ago`;
        return `${seconds}s ago`;
    }

    /**
     * Hide loading screen
     */
    function hideLoadingScreen() {
        setTimeout(() => {
            if (cache.loadingScreen) {
                cache.loadingScreen.classList.add('hidden');
                setTimeout(() => {
                    cache.loadingScreen.style.display = 'none';
                }, 300);
            }
        }, 1000);
    }

    // Browser history support
    window.addEventListener('popstate', (e) => {
        if (e.state && e.state.url) {
            navigateTo(e.state.url, false);
        }
    });

    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }

    // Expose public API
    window.EnvyRemote = {
        navigateTo,
        refreshCurrentPage,
        showNotification,
        ajaxRequest
    };

})();
